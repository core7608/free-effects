#include "tga_importer.h"
#include <cstring>
#include <algorithm>

namespace FreeEffect {

bool TGAImporter::isTGAFile(const std::string& filePath) {
    size_t dotPos = filePath.rfind('.');
    if (dotPos == std::string::npos) return false;
    std::string ext = filePath.substr(dotPos);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return ext == ".tga" || ext == ".vda" || ext == ".icb" || ext == ".vst";
}

bool TGAImporter::open(const std::string& filePath) {
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

bool TGAImporter::readHeader() {
    if (m_rawData.size() < 18) {
        m_errorMessage = "File too small for TGA header";
        return false;
    }

    const uint8_t* hdr = m_rawData.data();
    m_header.idLength = hdr[0];
    m_header.colorMapType = hdr[1];
    m_header.imageType = hdr[2];
    m_header.colorMapOrigin = hdr[3] | (static_cast<uint16_t>(hdr[4]) << 8);
    m_header.colorMapLength = hdr[5] | (static_cast<uint16_t>(hdr[6]) << 8);
    m_header.colorMapEntrySize = hdr[7];
    m_header.xOrigin = hdr[8] | (static_cast<uint16_t>(hdr[9]) << 8);
    m_header.yOrigin = hdr[10] | (static_cast<uint16_t>(hdr[11]) << 8);
    m_header.width = hdr[12] | (static_cast<uint16_t>(hdr[13]) << 8);
    m_header.height = hdr[14] | (static_cast<uint16_t>(hdr[15]) << 8);
    m_header.bitsPerPixel = hdr[16];
    m_header.imageDescriptor = hdr[17];

    m_header.topToBottom = (m_header.imageDescriptor & 0x20) != 0;
    m_header.leftToRight = true;

    int alphaBits = (m_header.imageDescriptor >> 4) & 0x0F;
    m_header.hasAlpha = (alphaBits > 0) || (m_header.bitsPerPixel == 32);

    m_header.imageId.clear();
    if (m_header.idLength > 0 && 18 + m_header.idLength <= m_rawData.size()) {
        m_header.imageId.assign(reinterpret_cast<const char*>(hdr + 18), m_header.idLength);
    }

    m_header.isRLE = (m_header.imageType == 9 || m_header.imageType == 10 || m_header.imageType == 11);

    if (m_header.width == 0 || m_header.height == 0) {
        m_errorMessage = "Invalid TGA dimensions";
        return false;
    }

    m_headerRead = true;
    return true;
}

bool TGAImporter::readPixels(TGAImageData& imageData) {
    if (!m_headerRead) {
        m_errorMessage = "Header not read yet";
        return false;
    }

    int w = m_header.width;
    int h = m_header.height;
    int bpp = m_header.bitsPerPixel;
    int srcBPP = bpp / 8;

    imageData.width = w;
    imageData.height = h;
    imageData.bytesPerPixel = 4;

    size_t dataOffset = 18 + m_header.idLength;
    size_t colorMapSize = 0;

    if (m_header.colorMapType == 1) {
        colorMapSize = static_cast<size_t>(m_header.colorMapLength) * ((m_header.colorMapEntrySize + 7) / 8);
        dataOffset += colorMapSize;
    }

    std::vector<uint8_t> colorMap;
    if (m_header.colorMapType == 1 && dataOffset <= m_rawData.size()) {
        size_t cmStart = 18 + m_header.idLength;
        int entryBytes = (m_header.colorMapEntrySize + 7) / 8;
        colorMap.resize(static_cast<size_t>(m_header.colorMapLength) * 3);
        for (int i = 0; i < m_header.colorMapLength; ++i) {
            size_t srcOff = cmStart + static_cast<size_t>(i) * entryBytes;
            if (srcOff + entryBytes <= m_rawData.size()) {
                if (entryBytes == 3) {
                    colorMap[i * 3 + 0] = m_rawData[srcOff + 0];
                    colorMap[i * 3 + 1] = m_rawData[srcOff + 1];
                    colorMap[i * 3 + 2] = m_rawData[srcOff + 2];
                } else if (entryBytes == 4) {
                    colorMap[i * 3 + 0] = m_rawData[srcOff + 1];
                    colorMap[i * 3 + 1] = m_rawData[srcOff + 2];
                    colorMap[i * 3 + 2] = m_rawData[srcOff + 3];
                }
            }
        }
    }

    std::vector<uint8_t> rawPixels;
    const uint8_t* pixelData = nullptr;
    size_t pixelDataSize = 0;

    if (dataOffset < m_rawData.size()) {
        pixelData = m_rawData.data() + dataOffset;
        pixelDataSize = m_rawData.size() - dataOffset;
    }

    if (m_header.isRLE) {
        rawPixels.clear();
        size_t pos = 0;
        int totalPixels = w * h;
        int pixelsRead = 0;

        while (pixelsRead < totalPixels && pos < pixelDataSize) {
            uint8_t header = pixelData[pos++];
            int count = (header & 0x7F) + 1;

            if (header & 0x80) {
                if (pos + srcBPP > pixelDataSize) break;
                for (int i = 0; i < count && pixelsRead < totalPixels; ++i) {
                    rawPixels.insert(rawPixels.end(), pixelData + pos, pixelData + pos + srcBPP);
                    pixelsRead++;
                }
                pos += srcBPP;
            } else {
                int runLength = count;
                if (pos + runLength * srcBPP > pixelDataSize) break;
                for (int i = 0; i < runLength && pixelsRead < totalPixels; ++i) {
                    rawPixels.insert(rawPixels.end(), pixelData + pos, pixelData + pos + srcBPP);
                    pixelsRead++;
                    pos += srcBPP;
                }
            }
        }
        pixelData = rawPixels.data();
        pixelDataSize = rawPixels.size();
    }

    imageData.pixels.resize(static_cast<size_t>(w) * h * 4, 0);

    size_t srcPos = 0;
    for (int y = 0; y < h; ++y) {
        int destY = m_header.topToBottom ? y : (h - 1 - y);

        for (int x = 0; x < w; ++x) {
            if (srcPos + srcBPP > pixelDataSize) break;

            uint8_t b = 0, g = 0, r = 0, a = 255;

            switch (bpp) {
                case 8:
                    if (m_header.colorMapType == 1) {
                        uint8_t idx = pixelData[srcPos];
                        if (idx < m_header.colorMapLength) {
                            r = colorMap[idx * 3 + 0];
                            g = colorMap[idx * 3 + 1];
                            b = colorMap[idx * 3 + 2];
                        }
                    } else {
                        r = g = b = pixelData[srcPos];
                    }
                    break;
                case 16:
                    b = pixelData[srcPos];
                    g = pixelData[srcPos + 1];
                    a = (g & 0x80) ? 255 : 0;
                    r = (g & 0x7F) << 1;
                    g = ((b & 0xF0) >> 3) | ((g & 0x0F) << 5);
                    b = (b & 0x1F) << 3;
                    break;
                case 24:
                    b = pixelData[srcPos];
                    g = pixelData[srcPos + 1];
                    r = pixelData[srcPos + 2];
                    break;
                case 32:
                    b = pixelData[srcPos];
                    g = pixelData[srcPos + 1];
                    r = pixelData[srcPos + 2];
                    a = pixelData[srcPos + 3];
                    break;
                default:
                    break;
            }

            size_t destIdx = (static_cast<size_t>(destY) * w + x) * 4;
            if (destIdx + 3 < imageData.pixels.size()) {
                imageData.pixels[destIdx + 0] = r;
                imageData.pixels[destIdx + 1] = g;
                imageData.pixels[destIdx + 2] = b;
                imageData.pixels[destIdx + 3] = a;
            }

            srcPos += srcBPP;
        }
    }

    return true;
}

void TGAImporter::close() {
    if (m_file) {
        fclose(m_file);
        m_file = nullptr;
    }
    m_fileOpen = false;
    m_headerRead = false;
    m_rawData.clear();
}

} // namespace FreeEffect
