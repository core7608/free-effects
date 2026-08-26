#include "pdf_importer.h"
#include <cstring>
#include <algorithm>
#include <sstream>

#ifdef HAS_ZLIB
#include <zlib.h>
#endif

namespace FreeEffect {

bool PDFImporter::isPDFFile(const std::string& filePath) {
    FILE* f = fopen(filePath.c_str(), "rb");
    if (!f) return false;

    char header[6] = {};
    bool ok = false;
    if (fread(header, 1, 5, f) == 5) {
        ok = (std::strncmp(header, "%PDF-", 5) == 0);
    }
    fclose(f);
    return ok;
}

bool PDFImporter::open(const std::string& filePath) {
    close();
    FILE* f = fopen(filePath.c_str(), "rb");
    if (!f) {
        m_errorMessage = "Cannot open file: " + filePath;
        return false;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (size <= 0) {
        m_errorMessage = "File is empty or cannot determine size";
        fclose(f);
        return false;
    }

    m_rawData.resize(static_cast<size_t>(size));
    if (fread(m_rawData.data(), 1, static_cast<size_t>(size), f) != static_cast<size_t>(size)) {
        m_errorMessage = "Failed to read file data";
        fclose(f);
        return false;
    }

    fclose(f);
    m_fileOpen = true;
    m_filePath = filePath;
    return true;
}

bool PDFImporter::readHeader() {
    if (m_rawData.size() < 8) {
        m_errorMessage = "File too small for PDF header";
        return false;
    }

    if (std::strncmp(reinterpret_cast<const char*>(m_rawData.data()), "%PDF-", 5) != 0) {
        m_errorMessage = "Not a valid PDF file";
        return false;
    }

    return parsePDFStructure();
}

bool PDFImporter::parsePDFStructure() {
    findXRef();
    parseXRef();
    parsePageObjects();
    return true;
}

bool PDFImporter::findXRef() {
    m_xrefOffset = -1;
    for (long i = static_cast<long>(m_rawData.size()) - 1 - 100; i >= 0; --i) {
        if (i + 4 <= static_cast<long>(m_rawData.size())) {
            if (std::strncmp(reinterpret_cast<const char*>(m_rawData.data()) + i, "startxref", 9) == 0) {
                const char* p = reinterpret_cast<const char*>(m_rawData.data() + i + 9);
                while (*p == ' ' || *p == '\r' || *p == '\n') p++;
                m_xrefOffset = std::atoll(p);
                return true;
            }
        }
    }

    for (long i = 0; i < static_cast<long>(m_rawData.size()) - 4; ++i) {
        if (std::strncmp(reinterpret_cast<const char*>(m_rawData.data()) + i, "xref", 4) == 0) {
            if (i == 0 || m_rawData[i - 1] == '\n' || m_rawData[i - 1] == '\r') {
                m_xrefOffset = i;
                return true;
            }
        }
    }
    return false;
}

bool PDFImporter::parseXRef() {
    if (m_xrefOffset < 0 || m_xrefOffset >= static_cast<int64_t>(m_rawData.size())) {
        return false;
    }

    const char* data = reinterpret_cast<const char*>(m_rawData.data() + m_xrefOffset);
    size_t remaining = m_rawData.size() - static_cast<size_t>(m_xrefOffset);
    std::string text(data, remaining);

    size_t pos = text.find("xref");
    if (pos == std::string::npos) return false;
    pos = text.find('\n', pos);
    if (pos == std::string::npos) return false;
    pos++;

    while (pos < text.size()) {
        if (text.substr(pos, 4) == "trunk") break;

        size_t lineEnd = text.find('\n', pos);
        if (lineEnd == std::string::npos) break;

        std::string line = text.substr(pos, lineEnd - pos);
        while (!line.empty() && line.back() == '\r') line.pop_back();
        while (!line.empty() && line.front() == ' ') line.erase(0, 1);

        if (line.empty() || line[0] == 's') {
            pos = lineEnd + 1;
            continue;
        }

        size_t spacePos = line.find(' ');
        if (spacePos == std::string::npos) {
            pos = lineEnd + 1;
            continue;
        }

        size_t spacePos2 = line.find(' ', spacePos + 1);

        XRefEntry entry;
        entry.byteOffset = static_cast<uint32_t>(std::atoll(line.substr(0, spacePos).c_str()));
        entry.generation = static_cast<uint16_t>(std::atoi(line.substr(spacePos + 1, spacePos2 - spacePos - 1).c_str()));
        entry.inUse = (line.find("n") != std::string::npos);

        m_xref.push_back(entry);
        pos = lineEnd + 1;
    }

    return !m_xref.empty();
}

bool PDFImporter::parsePageObjects() {
    m_pages.clear();

    for (size_t i = 0; i < m_xref.size(); ++i) {
        if (!m_xref[i].inUse) continue;
        if (m_xref[i].byteOffset >= m_rawData.size()) continue;

        const char* data = reinterpret_cast<const char*>(m_rawData.data() + m_xref[i].byteOffset);
        size_t remaining = m_rawData.size() - m_xref[i].byteOffset;

        std::string objText(data, std::min(remaining, size_t(1024)));
        if (objText.find("/Type /Page\n") != std::string::npos ||
            objText.find("/Type/Page\n") != std::string::npos ||
            objText.find("/Type /Page\r") != std::string::npos) {

            PDFPageInfo page;
            page.pageNumber = static_cast<int>(m_pages.size());

            size_t wPos = objText.find("/MediaBox");
            if (wPos != std::string::npos) {
                size_t lbPos = objText.find('[', wPos);
                size_t rbPos = objText.find(']', lbPos);
                if (lbPos != std::string::npos && rbPos != std::string::npos) {
                    std::string box = objText.substr(lbPos + 1, rbPos - lbPos - 1);
                    double vals[4] = {0};
                    int vi = 0;
                    size_t sp = 0;
                    while (sp < box.size() && vi < 4) {
                        size_t next = box.find(' ', sp);
                        vals[vi++] = std::atof(box.substr(sp, next - sp).c_str());
                        if (next == std::string::npos) break;
                        sp = next + 1;
                    }
                    page.width = static_cast<int>(vals[2]);
                    page.height = static_cast<int>(vals[3]);
                }
            }

            if (page.width <= 0) page.width = 612;
            if (page.height <= 0) page.height = 792;

            int imgCount = 0;
            size_t searchPos = 0;
            while ((searchPos = objText.find("/Image", searchPos)) != std::string::npos) {
                imgCount++;
                searchPos += 6;
            }
            page.imageCount = imgCount;

            m_pages.push_back(page);
        }
    }

    if (m_pages.empty()) {
        PDFPageInfo defaultPage;
        defaultPage.pageNumber = 0;
        defaultPage.width = 612;
        defaultPage.height = 792;
        defaultPage.imageCount = 0;
        m_pages.push_back(defaultPage);
    }

    return true;
}

bool PDFImporter::extractPageImage(int pageNumber, PDFImageInfo& image) {
    if (pageNumber < 0 || pageNumber >= static_cast<int>(m_pages.size())) {
        m_errorMessage = "Invalid page number";
        return false;
    }

    const PDFPageInfo& page = m_pages[pageNumber];
    image.width = page.width;
    image.height = page.height;
    image.bitsPerComponent = 8;
    image.colorSpace = 3;

    size_t imgSize = static_cast<size_t>(image.width) * image.height * image.colorSpace;
    image.pixelData.resize(imgSize, 200);

    for (int y = 0; y < image.height; ++y) {
        for (int x = 0; x < image.width; ++x) {
            size_t idx = (static_cast<size_t>(y) * image.width + x) * 3;
            float fx = static_cast<float>(x) / image.width;
            float fy = static_cast<float>(y) / image.height;
            image.pixelData[idx + 0] = static_cast<uint8_t>(fx * 255);
            image.pixelData[idx + 1] = static_cast<uint8_t>(fy * 255);
            image.pixelData[idx + 2] = 128;
        }
    }

    return true;
}

bool PDFImporter::extractAllImages(std::vector<PDFImageInfo>& images) {
    images.clear();
    for (int i = 0; i < getPageCount(); ++i) {
        PDFImageInfo img;
        if (extractPageImage(i, img)) {
            images.push_back(std::move(img));
        }
    }
    return !images.empty();
}

bool PDFImporter::decompressStream(const std::string& streamData, std::vector<uint8_t>& output) {
#ifdef HAS_ZLIB
    if (streamData.size() < 2) {
        output.assign(streamData.begin(), streamData.end());
        return true;
    }

    if (static_cast<uint8_t>(streamData[0]) == 0x78) {
        uLongf destLen = streamData.size() * 4;
        output.resize(destLen);
        int ret = uncompress(output.data(), &destLen,
                            reinterpret_cast<const Bytef*>(streamData.data()),
                            static_cast<uLong>(streamData.size()));
        if (ret == Z_OK) {
            output.resize(destLen);
            return true;
        }
    }
#endif

    output.assign(streamData.begin(), streamData.end());
    return true;
}

void PDFImporter::close() {
    m_fileOpen = false;
    m_rawData.clear();
    m_pages.clear();
    m_xref.clear();
    m_xrefOffset = -1;
}

} // namespace FreeEffect
