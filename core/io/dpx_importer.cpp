#include "dpx_importer.h"
#include <cstring>
#include <algorithm>
#include <cmath>

namespace FreeEffect {

bool DPXImporter::isDPXFile(const std::string& filePath) {
    FILE* f = fopen(filePath.c_str(), "rb");
    if (!f) return false;

    uint8_t magic[4];
    bool ok = false;
    if (fread(magic, 1, 4, f) == 4) {
        ok = (magic[0] == 'S' && magic[1] == 'D' && magic[2] == 'P' && magic[3] == 'X') ||
             (magic[0] == 'D' && magic[1] == 'P' && magic[2] == 'X' && magic[3] == 'S');
    }
    fclose(f);
    return ok;
}

uint16_t DPXImporter::readU16(const uint8_t* p) const {
    if (m_header.isLittleEndian)
        return static_cast<uint16_t>(p[0]) | (static_cast<uint16_t>(p[1]) << 8);
    else
        return (static_cast<uint16_t>(p[0]) << 8) | static_cast<uint16_t>(p[1]);
}

uint32_t DPXImporter::readU32(const uint8_t* p) const {
    if (m_header.isLittleEndian)
        return static_cast<uint32_t>(p[0]) | (static_cast<uint32_t>(p[1]) << 8) |
               (static_cast<uint32_t>(p[2]) << 16) | (static_cast<uint32_t>(p[3]) << 24);
    else
        return (static_cast<uint32_t>(p[0]) << 24) | (static_cast<uint32_t>(p[1]) << 16) |
               (static_cast<uint32_t>(p[2]) << 8) | static_cast<uint32_t>(p[3]);
}

bool DPXImporter::open(const std::string& filePath) {
    close();
    m_file = fopen(filePath.c_str(), "rb");
    if (!m_file) {
        m_errorMessage = "Cannot open file: " + filePath;
        return false;
    }
    m_fileOpen = true;
    m_filePath = filePath;

    fseek(m_file, 0, SEEK_END);
    long size = ftell(m_file);
    fseek(m_file, 0, SEEK_SET);

    if (size <= 0) {
        m_errorMessage = "File is empty or cannot determine size";
        close();
        return false;
    }

    m_rawData.resize(static_cast<size_t>(size));
    if (fread(m_rawData.data(), 1, static_cast<size_t>(size), m_file) != static_cast<size_t>(size)) {
        m_errorMessage = "Failed to read file data";
        close();
        return false;
    }

    fclose(m_file);
    m_file = nullptr;

    return true;
}

bool DPXImporter::readHeader() {
    if (m_rawData.size() < 644) {
        m_errorMessage = "File too small for DPX header";
        return false;
    }

    const uint8_t* hdr = m_rawData.data();

    if (hdr[0] == 'D' && hdr[1] == 'P' && hdr[2] == 'X' && hdr[3] == 'S') {
        m_header.isLittleEndian = false;
    } else if (hdr[0] == 'S' && hdr[1] == 'D' && hdr[2] == 'P' && hdr[3] == 'X') {
        m_header.isLittleEndian = true;
    } else {
        m_errorMessage = "Invalid DPX magic number";
        return false;
    }

    uint32_t fileSize = readU32(hdr + 4);
    uint32_t dittoKey = readU32(hdr + 8);
    uint32_t genericHeaderLength = readU32(hdr + 12);
    uint32_t indHdrLen = readU32(hdr + 16);
    uint32_t fileHdrLen = readU32(hdr + 20);

    m_header.filename.clear();
    if (hdr[36] != 0) {
        for (int i = 36; i < 72 && hdr[i] != 0; i++)
            m_header.filename += static_cast<char>(hdr[i]);
    }

    m_header.date.clear();
    if (hdr[72] != 0) {
        for (int i = 72; i < 92 && hdr[i] != 0; i++)
            m_header.date += static_cast<char>(hdr[i]);
    }

    m_header.creator.clear();
    if (hdr[128] != 0) {
        for (int i = 128; i < 164 && hdr[i] != 0; i++)
            m_header.creator += static_cast<char>(hdr[i]);
    }

    m_header.project.clear();
    if (hdr[164] != 0) {
        for (int i = 164; i < 200 && hdr[i] != 0; i++)
            m_header.project += static_cast<char>(hdr[i]);
    }

    m_header.copyright.clear();
    if (hdr[200] != 0) {
        for (int i = 200; i < hdr[244]; i++)
            m_header.copyright += static_cast<char>(hdr[i]);
    }

    uint32_t offsetToData = readU32(hdr + 244);
    m_header.descriptor = readU32(hdr + 256);
    m_header.transfer = readU32(hdr + 260);
    m_header.colorimetric = readU32(hdr + 264);
    m_header.bitsPerElement = readU32(hdr + 268);
    m_header.packing = static_cast<DPXPacking>(readU32(hdr + 272));
    m_header.storage = static_cast<DPXPixelStorage>(readU32(hdr + 276));
    m_header.orientation = static_cast<DPXOrientation>(readU32(hdr + 280));
    m_header.elementsPerPixel = static_cast<int>(readU32(hdr + 284));
    m_header.linePacking = static_cast<int>(readU32(hdr + 288));

    if (m_header.elementsPerPixel >= 1) {
        m_header.width = static_cast<int>(readU32(hdr + 292));
    }
    if (m_header.elementsPerPixel >= 1) {
        m_header.height = static_cast<int>(readU32(hdr + 296));
    }

    m_header.dataSign = readU32(hdr + 304);
    m_header.lowQuantity = 0.0f;
    m_header.highQuantity = 1.0f;

    if (hdr + 332 + 8 <= m_rawData.data() + m_rawData.size()) {
        uint32_t lo = readU32(hdr + 332);
        uint32_t hi = readU32(hdr + 336);
        std::memcpy(&m_header.lowQuantity, &lo, 4);
        std::memcpy(&m_header.highQuantity, &hi, 4);
    }

    m_header.dwordPadding = 0;
    if (m_header.linePacking > m_header.width * m_header.elementsPerPixel * ((m_header.bitsPerElement + 7) / 8)) {
        m_header.dwordPadding = m_header.linePacking - m_header.width * m_header.elementsPerPixel * ((m_header.bitsPerElement + 7) / 8);
    }

    m_littleEndian = m_header.isLittleEndian;

    if (m_header.width <= 0) m_header.width = 1920;
    if (m_header.height <= 0) m_header.height = 1080;

    m_headerRead = true;
    return true;
}

bool DPXImporter::readPixels(DPXImageData& imageData) {
    if (!m_headerRead) {
        m_errorMessage = "Header not read yet";
        return false;
    }

    int w = m_header.width;
    int h = m_header.height;
    imageData.width = w;
    imageData.height = h;
    imageData.bitsPerComponent = m_header.bitsPerElement;

    int srcBPC = m_header.bitsPerElement;
    int elements = m_header.elementsPerPixel;
    int bytesPerElement = (srcBPC + 7) / 8;
    int bytesPerPixel = elements * bytesPerElement;
    int lineSize = w * bytesPerPixel + static_cast<int>(m_header.dwordPadding);

    uint32_t offsetToData = 0;
    if (m_rawData.size() >= 248) {
        offsetToData = readU32(m_rawData.data() + 244);
    }
    if (offsetToData == 0 || offsetToData >= m_rawData.size()) {
        offsetToData = 644 + industryHeaderLength() + fileHeaderLength();
        if (offsetToData < 668) offsetToData = 668;
    }

    size_t dataStart = offsetToData;
    size_t dataAvail = m_rawData.size() > dataStart ? m_rawData.size() - dataStart : 0;
    size_t totalLines = dataAvail / (lineSize > 0 ? lineSize : 1);
    size_t linesToRead = std::min(totalLines, static_cast<size_t>(h));

    imageData.rgbPixels.resize(static_cast<size_t>(w) * h * 3, 0.0f);

    float maxVal = static_cast<float>((1 << srcBPC) - 1);
    if (maxVal <= 0) maxVal = 1.0f;

    for (int y = 0; y < static_cast<int>(linesToRead); ++y) {
        const uint8_t* lineStart = m_rawData.data() + dataStart + static_cast<size_t>(y) * lineSize;

        for (int x = 0; x < w; ++x) {
            const uint8_t* pixel = lineStart + static_cast<size_t>(x) * bytesPerPixel;

            float r = 0, g = 0, b = 0;

            if (srcBPC == 10 && m_header.packing == DPXPacking::PackWord1) {
                if (m_header.isLittleEndian) {
                    uint32_t packed = static_cast<uint32_t>(pixel[0]) |
                                     (static_cast<uint32_t>(pixel[1]) << 8) |
                                     (static_cast<uint32_t>(pixel[2]) << 16) |
                                     (static_cast<uint32_t>(pixel[3]) << 24);
                    if (elements >= 3) {
                        r = static_cast<float>(packed & 0x3FF) / 1023.0f;
                        g = static_cast<float>((packed >> 10) & 0x3FF) / 1023.0f;
                        b = static_cast<float>((packed >> 20) & 0x3FF) / 1023.0f;
                    }
                } else {
                    uint32_t packed = (static_cast<uint32_t>(pixel[0]) << 24) |
                                     (static_cast<uint32_t>(pixel[1]) << 16) |
                                     (static_cast<uint32_t>(pixel[2]) << 8) |
                                     static_cast<uint32_t>(pixel[3]);
                    if (elements >= 3) {
                        r = static_cast<float>((packed >> 22) & 0x3FF) / 1023.0f;
                        g = static_cast<float>((packed >> 12) & 0x3FF) / 1023.0f;
                        b = static_cast<float>((packed >> 2) & 0x3FF) / 1023.0f;
                    }
                }
            } else if (srcBPC == 8) {
                r = static_cast<float>(pixel[0]) / 255.0f;
                g = elements >= 2 ? static_cast<float>(pixel[1]) / 255.0f : r;
                b = elements >= 3 ? static_cast<float>(pixel[2]) / 255.0f : r;
            } else if (srcBPC == 16) {
                uint16_t rv = readU16(pixel);
                uint16_t gv = elements >= 2 ? readU16(pixel + 2) : rv;
                uint16_t bv = elements >= 3 ? readU16(pixel + 4) : rv;
                r = static_cast<float>(rv) / 65535.0f;
                g = static_cast<float>(gv) / 65535.0f;
                b = static_cast<float>(bv) / 65535.0f;
            } else if (srcBPC == 12) {
                if (m_header.isLittleEndian) {
                    uint32_t packed = static_cast<uint32_t>(pixel[0]) |
                                     (static_cast<uint32_t>(pixel[1]) << 8) |
                                     (static_cast<uint32_t>(pixel[2]) << 16);
                    r = static_cast<float>(packed & 0xFFF) / 4095.0f;
                    g = static_cast<float>((packed >> 12) & 0xFFF) / 4095.0f;
                    b = static_cast<float>((packed >> 24) & 0xFF) / 255.0f;
                }
            } else {
                r = g = b = 0;
            }

            size_t pi = (static_cast<size_t>(y) * w + x) * 3;
            imageData.rgbPixels[pi + 0] = r;
            imageData.rgbPixels[pi + 1] = g;
            imageData.rgbPixels[pi + 2] = b;
        }
    }

    return true;
}

void DPXImporter::close() {
    if (m_file) {
        fclose(m_file);
        m_file = nullptr;
    }
    m_fileOpen = false;
    m_headerRead = false;
    m_rawData.clear();
}

uint32_t DPXImporter::industryHeaderLength() const {
    return 0;
}

uint32_t DPXImporter::fileHeaderLength() const {
    return 0;
}

} // namespace FreeEffect
