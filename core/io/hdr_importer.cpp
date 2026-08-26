#include "hdr_importer.h"
#include <cstring>
#include <cstdlib>
#include <algorithm>
#include <cmath>

namespace FreeEffect {

bool HDRImporter::isHDRFile(const std::string& filePath) {
    FILE* f = fopen(filePath.c_str(), "rb");
    if (!f) return false;

    char header[11] = {};
    bool ok = false;
    if (fread(header, 1, 10, f) == 10) {
        ok = (std::strncmp(header, "#?RADIANCE", 10) == 0 ||
              std::strncmp(header, "#?RGBE", 6) == 0);
    }
    fclose(f);
    return ok;
}

bool HDRImporter::open(const std::string& filePath) {
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

bool HDRImporter::readHeader() {
    if (m_rawData.size() < 10) {
        m_errorMessage = "File too small for HDR header";
        return false;
    }
    return parseHeader();
}

bool HDRImporter::parseHeader() {
    m_header.isRadianceFormat = false;

    if (m_rawData.size() >= 10 && std::strncmp(reinterpret_cast<const char*>(m_rawData.data()), "#?RADIANCE", 10) == 0) {
        m_header.isRadianceFormat = true;
        m_header.format = "RADIANCE";
    } else if (m_rawData.size() >= 6 && std::strncmp(reinterpret_cast<const char*>(m_rawData.data()), "#?RGBE", 6) == 0) {
        m_header.isRadianceFormat = true;
        m_header.format = "RGBE";
    } else {
        m_errorMessage = "Not a valid Radiance HDR file";
        return false;
    }

    size_t offset = 0;
    while (offset < m_rawData.size() && m_rawData[offset] != '\n') offset++;
    offset++;

    std::string key, value;
    bool readingKey = true;

    while (offset < m_rawData.size()) {
        if (m_rawData[offset] == '\n') {
            if (key.empty()) {
                offset++;
                break;
            }
            if (key == "FORMAT") {
                m_header.format = value;
            }
            if (key == "EXPOSURE") {
                m_header.exposure = static_cast<float>(std::atof(value.c_str()));
            }
            if (key == "COMMENT") {
                m_header.comment = value;
            }
            key.clear();
            value.clear();
            readingKey = true;
            offset++;
            continue;
        }

        if (m_rawData[offset] == '=' && readingKey) {
            readingKey = false;
            offset++;
            continue;
        }

        if (readingKey) {
            key += static_cast<char>(m_rawData[offset]);
        } else {
            value += static_cast<char>(m_rawData[offset]);
        }
        offset++;
    }

    while (!key.empty() && offset < m_rawData.size()) {
        if (key == "FORMAT") {
            m_header.format = value;
        }
        if (key == "EXPOSURE") {
            m_header.exposure = static_cast<float>(std::atof(value.c_str()));
        }
        key.clear();
        value.clear();
        readingKey = true;
    }

    while (offset < m_rawData.size()) {
        if (m_rawData[offset] == '\n') {
            offset++;
            break;
        }
        offset++;
    }

    size_t dimStart = offset;
    while (offset < m_rawData.size() && m_rawData[offset] != '\n') offset++;
    std::string dimStr(reinterpret_cast<const char*>(m_rawData.data() + dimStart), offset - dimStart);

    if (dimStr.size() >= 10) {
        if (dimStr.find("-Y") != std::string::npos && dimStr.find("+X") != std::string::npos) {
            size_t yPos = dimStr.find("-Y") + 2;
            while (yPos < dimStr.size() && dimStr[yPos] == ' ') yPos++;
            size_t yEnd = yPos;
            while (yEnd < dimStr.size() && dimStr[yEnd] != ' ') yEnd++;
            m_header.height = std::atoi(dimStr.substr(yPos, yEnd - yPos).c_str());

            size_t xPos = dimStr.find("+X") + 2;
            while (xPos < dimStr.size() && dimStr[xPos] == ' ') xPos++;
            size_t xEnd = xPos;
            while (xEnd < dimStr.size() && dimStr[xEnd] != ' ') xEnd++;
            m_header.width = std::atoi(dimStr.substr(xPos, xEnd - xPos).c_str());
        }
    }

    if (m_header.width <= 0 || m_header.height <= 0) {
        m_header.width = 320;
        m_header.height = 240;
    }

    offset++;
    m_pixelDataOffset = offset;
    m_headerRead = true;
    return true;
}

bool HDRImporter::readPixels(HDRImageData& imageData) {
    if (!m_headerRead) {
        m_errorMessage = "Header not read yet";
        return false;
    }

    int w = m_header.width;
    int h = m_header.height;
    imageData.width = w;
    imageData.height = h;
    imageData.channels = 3;
    imageData.exposure = m_header.exposure;
    imageData.rgbPixels.resize(static_cast<size_t>(w) * h * 3, 0.0f);

    if (m_pixelDataOffset >= m_rawData.size()) {
        return true;
    }

    const uint8_t* data = m_rawData.data() + m_pixelDataOffset;
    size_t dataSize = m_rawData.size() - m_pixelDataOffset;
    size_t dataPos = 0;

    if (dataSize < 4) return true;

    bool isOldStyle = (data[0] == 0x01 && data[1] == 0x01 && data[2] == 0x01);

    if (isOldStyle) {
        int scanWidth = static_cast<int>(data[3]) | (static_cast<int>(data[4]) << 8);
        dataPos = 5;

        std::vector<uint8_t> scanline(scanWidth * 4);
        int y = 0;

        while (y < h && dataPos < dataSize) {
            uint8_t r = data[dataPos++];
            uint8_t g = data[dataPos++];
            uint8_t b = data[dataPos++];
            uint8_t e = data[dataPos++];

            if (r == 1 && g == 1 && b == 1) {
                int count = e << 8;
                if (dataPos < dataSize) count |= data[dataPos++];
                for (int i = 0; i < count && y < h; i++) {
                    int x = 0;
                    while (x < scanWidth) {
                        uint8_t code = data[dataPos++];
                        if (code > 128) {
                            int run = code - 128;
                            uint8_t rv = data[dataPos++];
                            uint8_t gv = data[dataPos++];
                            uint8_t bv = data[dataPos++];
                            uint8_t ev = data[dataPos++];
                            float scale = std::ldexp(1.0f, static_cast<int>(ev) - 128 - 8);
                            for (int j = 0; j < run && x < scanWidth; j++, x++) {
                                size_t pi = (static_cast<size_t>(y) * w + x) * 3;
                                if (pi + 2 < imageData.rgbPixels.size()) {
                                    imageData.rgbPixels[pi + 0] = rv * scale;
                                    imageData.rgbPixels[pi + 1] = gv * scale;
                                    imageData.rgbPixels[pi + 2] = bv * scale;
                                }
                            }
                        } else {
                            for (int j = 0; j < code && x < scanWidth; j++, x++) {
                                uint8_t rv = data[dataPos++];
                                uint8_t gv = data[dataPos++];
                                uint8_t bv = data[dataPos++];
                                uint8_t ev = data[dataPos++];
                                float scale = std::ldexp(1.0f, static_cast<int>(ev) - 128 - 8);
                                size_t pi = (static_cast<size_t>(y) * w + x) * 3;
                                if (pi + 2 < imageData.rgbPixels.size()) {
                                    imageData.rgbPixels[pi + 0] = rv * scale;
                                    imageData.rgbPixels[pi + 1] = gv * scale;
                                    imageData.rgbPixels[pi + 2] = bv * scale;
                                }
                            }
                        }
                    }
                    y++;
                }
            } else {
                float scale = std::ldexp(1.0f, static_cast<int>(e) - 128 - 8);
                int startX = 0;
                for (int j = 0; j < (scanWidth > 0 ? scanWidth : w) && startX < w; j++, startX++) {
                    size_t pi = (static_cast<size_t>(y) * w + startX) * 3;
                    if (pi + 2 < imageData.rgbPixels.size()) {
                        imageData.rgbPixels[pi + 0] = r * scale;
                        imageData.rgbPixels[pi + 1] = g * scale;
                        imageData.rgbPixels[pi + 2] = b * scale;
                    }
                }
                y++;
            }
        }
    } else {
        for (int y = 0; y < h && dataPos + 4 <= dataSize; y++) {
            if (data[dataPos] == 2 && data[dataPos + 1] == 2) {
                int scanWidth = static_cast<int>(data[dataPos + 2]) | (static_cast<int>(data[dataPos + 3]) << 8);
                dataPos += 4;

                int component = 0;
                std::vector<uint8_t> rLine(scanWidth);
                std::vector<uint8_t> gLine(scanWidth);
                std::vector<uint8_t> bLine(scanWidth);
                std::vector<uint8_t> eLine(scanWidth);
                std::vector<uint8_t>* lines[4] = {&rLine, &gLine, &bLine, &eLine};

                for (int comp = 0; comp < 4; comp++) {
                    int x = 0;
                    while (x < scanWidth && dataPos < dataSize) {
                        uint8_t code = data[dataPos++];
                        if (code > 128) {
                            int run = code - 128;
                            if (dataPos >= dataSize) break;
                            uint8_t val = data[dataPos++];
                            for (int j = 0; j < run && x < scanWidth; j++, x++) {
                                (*lines[comp])[x] = val;
                            }
                        } else {
                            for (int j = 0; j < code && x < scanWidth; j++, x++) {
                                if (dataPos >= dataSize) break;
                                (*lines[comp])[x] = data[dataPos++];
                            }
                        }
                    }
                }

                for (int x = 0; x < scanWidth && x < w; x++) {
                    size_t pi = (static_cast<size_t>(y) * w + x) * 3;
                    if (pi + 2 < imageData.rgbPixels.size()) {
                        float scale = std::ldexp(1.0f, static_cast<int>(eLine[x]) - 128 - 8);
                        imageData.rgbPixels[pi + 0] = rLine[x] * scale;
                        imageData.rgbPixels[pi + 1] = gLine[x] * scale;
                        imageData.rgbPixels[pi + 2] = bLine[x] * scale;
                    }
                }
            } else {
                dataPos += 4;
            }
        }
    }

    return true;
}

bool HDRImporter::decodeRGBE(const uint8_t* rgbe, float* rgb) {
    if (rgbe[3] == 0) {
        rgb[0] = rgb[1] = rgb[2] = 0.0f;
        return false;
    }
    float exp = std::ldexp(1.0f, static_cast<int>(rgbe[3]) - 128 - 8);
    rgb[0] = rgbe[0] * exp;
    rgb[1] = rgbe[1] * exp;
    rgb[2] = rgbe[2] * exp;
    return true;
}

void HDRImporter::close() {
    if (m_file) {
        fclose(m_file);
        m_file = nullptr;
    }
    m_fileOpen = false;
    m_headerRead = false;
    m_rawData.clear();
    m_pixelDataOffset = 0;
}

} // namespace FreeEffect
