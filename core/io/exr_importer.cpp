#include "exr_importer.h"
#include <cstring>
#include <algorithm>
#include <cmath>

#ifdef HAS_ZLIB
#include <zlib.h>
#endif

namespace FreeEffect {

static const uint32_t OPENEXR_MAGIC = 0x762F3101;
static const uint16_t EXR_VERSION = 2;

static uint16_t readU16(const uint8_t* p) {
    return static_cast<uint16_t>(p[0]) | (static_cast<uint16_t>(p[1]) << 8);
}

static uint32_t readU32(const uint8_t* p) {
    return static_cast<uint32_t>(p[0]) | (static_cast<uint32_t>(p[1]) << 8) |
           (static_cast<uint32_t>(p[2]) << 16) | (static_cast<uint32_t>(p[3]) << 24);
}

static int32_t readI32(const uint8_t* p) {
    return static_cast<int32_t>(readU32(p));
}

static uint64_t readU64(const uint8_t* p) {
    uint64_t v = 0;
    for (int i = 0; i < 8; ++i)
        v |= static_cast<uint64_t>(p[i]) << (i * 8);
    return v;
}

static float readF32(const uint8_t* p) {
    float f;
    uint32_t u = readU32(p);
    std::memcpy(&f, &u, 4);
    return f;
}

bool EXRImporter::isEXRFile(const std::string& filePath) {
    FILE* f = fopen(filePath.c_str(), "rb");
    if (!f) return false;

    uint8_t magic[4];
    bool ok = false;
    if (fread(magic, 1, 4, f) == 4) {
        uint32_t val = static_cast<uint32_t>(magic[0]) | (static_cast<uint32_t>(magic[1]) << 8) |
                       (static_cast<uint32_t>(magic[2]) << 16) | (static_cast<uint32_t>(magic[3]) << 24);
        ok = (val == OPENEXR_MAGIC);
    }
    fclose(f);
    return ok;
}

float EXRImporter::halfToFloat(uint16_t halfVal) {
    uint32_t sign = (halfVal >> 15) & 1;
    uint32_t exponent = (halfVal >> 10) & 0x1F;
    uint32_t mantissa = halfVal & 0x3FF;

    uint32_t f;
    if (exponent == 0) {
        if (mantissa == 0) {
            f = sign << 31;
        } else {
            exponent = 1;
            while ((mantissa & 0x400) == 0) {
                mantissa <<= 1;
                exponent--;
            }
            mantissa &= 0x3FF;
            f = (sign << 31) | ((exponent + 127 - 15) << 23) | (mantissa << 13);
        }
    } else if (exponent == 31) {
        f = (sign << 31) | 0x7F800000 | (mantissa << 13);
    } else {
        f = (sign << 31) | ((exponent + 127 - 15) << 23) | (mantissa << 13);
    }

    float result;
    std::memcpy(&result, &f, 4);
    return result;
}

uint16_t EXRImporter::floatToHalf(float val) {
    uint32_t u;
    std::memcpy(&u, &val, 4);
    uint32_t sign = (u >> 16) & 0x8000;
    int32_t exponent = static_cast<int32_t>((u >> 23) & 0xFF) - 127 + 15;
    uint32_t mantissa = (u >> 13) & 0x3FF;

    if (exponent <= 0) {
        if (exponent < -10) return static_cast<uint16_t>(sign);
        mantissa = (mantissa | 0x400) >> (1 - exponent);
        return static_cast<uint16_t>(sign | (mantissa >> 1));
    } else if (exponent == 0xFF - 127 + 15) {
        return static_cast<uint16_t>(sign | 0x7C00 | (mantissa ? 0x200 : 0));
    } else if (exponent > 30) {
        return static_cast<uint16_t>(sign | 0x7C00);
    }
    return static_cast<uint16_t>(sign | (exponent << 10) | mantissa);
}

bool EXRImporter::open(const std::string& filePath) {
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

    if (size < 0) {
        m_errorMessage = "Cannot determine file size";
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

bool EXRImporter::readHeader() {
    if (m_rawData.size() < 12) {
        m_errorMessage = "File too small for EXR header";
        return false;
    }

    if (!readMagicNumber()) return false;
    if (!readVersion()) return false;
    if (!readAttributes()) return false;

    m_headerRead = true;
    return true;
}

bool EXRImporter::readMagicNumber() {
    uint32_t magic = readU32(m_rawData.data());
    if (magic != OPENEXR_MAGIC) {
        m_errorMessage = "Invalid EXR magic number";
        return false;
    }
    return true;
}

bool EXRImporter::readVersion() {
    uint16_t version = readU16(m_rawData.data() + 4);
    uint16_t flags = readU16(m_rawData.data() + 6);
    m_littleEndian = (flags & 0x02) != 0;
    m_header.isMultiPart = (flags & 0x10) != 0;
    m_header.isDeep = (flags & 0x20) != 0;
    return true;
}

bool EXRImporter::readAttributes() {
    size_t offset = 8;

    while (offset + 1 < m_rawData.size()) {
        if (m_rawData[offset] == 0) break;

        size_t nameStart = offset;
        while (offset < m_rawData.size() && m_rawData[offset] != 0) offset++;
        std::string attrName(reinterpret_cast<const char*>(m_rawData.data() + nameStart), offset - nameStart);
        offset++;

        size_t typeStart = offset;
        while (offset < m_rawData.size() && m_rawData[offset] != 0) offset++;
        std::string attrType(reinterpret_cast<const char*>(m_rawData.data() + typeStart), offset - typeStart);
        offset++;

        if (offset + 4 > m_rawData.size()) break;
        uint32_t dataSize = readU32(m_rawData.data() + offset);
        offset += 4;

        if (offset + dataSize > m_rawData.size()) break;
        const uint8_t* attrData = m_rawData.data() + offset;

        if (attrName == "channels") {
            m_header.channels.clear();
            size_t aOff = 0;
            while (aOff + 1 < dataSize) {
                size_t chNameStart = aOff;
                while (aOff < dataSize && attrData[aOff] != 0) aOff++;
                std::string chName(reinterpret_cast<const char*>(attrData + chNameStart), aOff - chNameStart);
                aOff++;
                if (aOff + 12 > dataSize) break;

                EXRChannelInfo ch;
                ch.name = chName;
                uint32_t pt = readU32(attrData + aOff);
                aOff += 4;
                ch.pixelType = static_cast<EXRPixelType>(pt);
                ch.xSampling = readI32(attrData + aOff);
                aOff += 4;
                ch.ySampling = readI32(attrData + aOff);
                aOff += 4;
                if (aOff < dataSize && attrData[aOff]) aOff++;
                aOff++;

                m_header.channels.push_back(ch);
            }
        } else if (attrName == "compression") {
            if (dataSize >= 1) {
                m_header.compression = static_cast<EXRCompression>(attrData[0]);
            }
        } else if (attrName == "dataWindow") {
            if (dataSize >= 16) {
                m_header.dataWindowXMin = readI32(attrData);
                m_header.dataWindowYMin = readI32(attrData + 4);
                m_header.dataWindowXMax = readI32(attrData + 8);
                m_header.dataWindowYMax = readI32(attrData + 12);
                m_header.width = m_header.dataWindowXMax - m_header.dataWindowXMin + 1;
                m_header.height = m_header.dataWindowYMax - m_header.dataWindowYMin + 1;
            }
        } else if (attrName == "displayWindow") {
            if (dataSize >= 16) {
                m_header.displayWindowXMin = readI32(attrData);
                m_header.displayWindowYMin = readI32(attrData + 4);
                m_header.displayWindowXMax = readI32(attrData + 8);
                m_header.displayWindowYMax = readI32(attrData + 12);
            }
        } else if (attrName == "pixelAspectRatio") {
            if (dataSize >= 4) {
                m_header.pixelAspectRatio = readF32(attrData);
            }
        } else if (attrName == "screenWindowWidth") {
            if (dataSize >= 4) {
                m_header.screenWindowWidth = readF32(attrData);
            }
        } else if (attrType != "string") {
            m_header.attributes.emplace_back(attrName, attrType);
        }

        offset += dataSize;
    }

    if (m_header.width <= 0 || m_header.height <= 0) {
        m_header.width = 1920;
        m_header.height = 1080;
    }

    return true;
}

bool EXRImporter::readChannelList() {
    return m_headerRead;
}

bool EXRImporter::readPixels(EXRImageData& imageData) {
    if (!m_headerRead) {
        m_errorMessage = "Header not read yet";
        return false;
    }

    int w = m_header.width;
    int h = m_header.height;
    imageData.width = w;
    imageData.height = h;
    imageData.rgbaPixels.resize(static_cast<size_t>(w) * h * 4, 0.0f);

    bool hasR = false, hasG = false, hasB = false, hasA = false;
    bool hasDepth = false, hasNormal = false;

    for (const auto& ch : m_header.channels) {
        if (ch.name == "R") hasR = true;
        else if (ch.name == "G") hasG = true;
        else if (ch.name == "B") hasB = true;
        else if (ch.name == "A") hasA = true;
        else if (ch.name == "Z" || ch.name == "depth") hasDepth = true;
        else if (ch.name == "N" || ch.name == "normal") hasNormal = true;
    }

    if (hasDepth) {
        imageData.hasDepth = true;
        imageData.depthPixels.resize(static_cast<size_t>(w) * h, 0.0f);
    }
    if (hasNormal) {
        imageData.hasNormals = true;
        imageData.normalPixels.resize(static_cast<size_t>(w) * h * 3, 0.0f);
    }

    size_t offset = 8;
    while (offset + 1 < m_rawData.size() && m_rawData[offset] != 0) {
        while (offset < m_rawData.size() && m_rawData[offset] != 0) offset++;
        offset++;
        while (offset < m_rawData.size() && m_rawData[offset] != 0) offset++;
        offset++;
        if (offset + 4 > m_rawData.size()) break;
        uint32_t sz = readU32(m_rawData.data() + offset);
        offset += 4;
        offset += sz;
    }
    if (offset < m_rawData.size() && m_rawData[offset] == 0) offset++;

    size_t pixelDataStart = offset;

    if (m_rawData.size() > pixelDataStart) {
        size_t availableData = m_rawData.size() - pixelDataStart;
        const uint8_t* src = m_rawData.data() + pixelDataStart;
        size_t bytesPerPixel = 0;
        for (const auto& ch : m_header.channels) {
            switch (ch.pixelType) {
                case EXRPixelType::Half: bytesPerPixel += 2; break;
                case EXRPixelType::Float: bytesPerPixel += 4; break;
                case EXRPixelType::UInt: bytesPerPixel += 4; break;
            }
        }
        size_t totalExpected = static_cast<size_t>(w) * h * bytesPerPixel;

        if (bytesPerPixel > 0 && availableData >= bytesPerPixel) {
            size_t pixelCount = std::min(availableData / bytesPerPixel, static_cast<size_t>(w) * h);

            int channelIdx = 0;
            int rIdx = -1, gIdx = -1, bIdx = -1, aIdx = -1;
            int depthIdx = -1, normIdx = -1;

            for (size_t i = 0; i < m_header.channels.size(); ++i) {
                const auto& ch = m_header.channels[i];
                if (ch.name == "R") rIdx = channelIdx;
                else if (ch.name == "G") gIdx = channelIdx;
                else if (ch.name == "B") bIdx = channelIdx;
                else if (ch.name == "A") aIdx = channelIdx;
                else if (ch.name == "Z" || ch.name == "depth") depthIdx = channelIdx;
                else if (ch.name == "N" || ch.name == "normal") normIdx = channelIdx;

                switch (ch.pixelType) {
                    case EXRPixelType::Half: channelIdx += 1; break;
                    case EXRPixelType::Float: channelIdx += 1; break;
                    case EXRPixelType::UInt: channelIdx += 1; break;
                }
            }

            size_t stride = 0;
            for (const auto& ch : m_header.channels) {
                switch (ch.pixelType) {
                    case EXRPixelType::Half: stride += 2; break;
                    case EXRPixelType::Float: stride += 4; break;
                    case EXRPixelType::UInt: stride += 4; break;
                }
            }

            auto readChannelValue = [&](const uint8_t* pixelPtr, EXRPixelType pt) -> float {
                switch (pt) {
                    case EXRPixelType::Half: return halfToFloat(readU16(pixelPtr));
                    case EXRPixelType::Float: return readF32(pixelPtr);
                    case EXRPixelType::UInt: return static_cast<float>(readU32(pixelPtr));
                }
                return 0.0f;
            };

            size_t channelOffset = 0;
            std::vector<size_t> channelOffsets;
            for (const auto& ch : m_header.channels) {
                channelOffsets.push_back(channelOffset);
                switch (ch.pixelType) {
                    case EXRPixelType::Half: channelOffset += 2; break;
                    case EXRPixelType::Float: channelOffset += 4; break;
                    case EXRPixelType::UInt: channelOffset += 4; break;
                }
            }

            for (size_t pi = 0; pi < pixelCount; ++pi) {
                const uint8_t* pixelPtr = src + pi * stride;

                if (rIdx >= 0 && rIdx < static_cast<int>(m_header.channels.size())) {
                    float r = readChannelValue(pixelPtr + channelOffsets[rIdx], m_header.channels[rIdx].pixelType);
                    size_t di = pi * 4;
                    imageData.rgbaPixels[di + 0] = std::clamp(r, 0.0f, 1.0f);
                }
                if (gIdx >= 0 && gIdx < static_cast<int>(m_header.channels.size())) {
                    float g = readChannelValue(pixelPtr + channelOffsets[gIdx], m_header.channels[gIdx].pixelType);
                    size_t di = pi * 4;
                    imageData.rgbaPixels[di + 1] = std::clamp(g, 0.0f, 1.0f);
                }
                if (bIdx >= 0 && bIdx < static_cast<int>(m_header.channels.size())) {
                    float b = readChannelValue(pixelPtr + channelOffsets[bIdx], m_header.channels[bIdx].pixelType);
                    size_t di = pi * 4;
                    imageData.rgbaPixels[di + 2] = std::clamp(b, 0.0f, 1.0f);
                }
                if (aIdx >= 0 && aIdx < static_cast<int>(m_header.channels.size())) {
                    float a = readChannelValue(pixelPtr + channelOffsets[aIdx], m_header.channels[aIdx].pixelType);
                    size_t di = pi * 4;
                    imageData.rgbaPixels[di + 3] = std::clamp(a, 0.0f, 1.0f);
                } else {
                    size_t di = pi * 4;
                    imageData.rgbaPixels[di + 3] = 1.0f;
                }

                if (hasDepth && depthIdx >= 0 && depthIdx < static_cast<int>(m_header.channels.size())) {
                    float d = readChannelValue(pixelPtr + channelOffsets[depthIdx], m_header.channels[depthIdx].pixelType);
                    imageData.depthPixels[pi] = d;
                }
            }
        }
    }

    if (imageData.rgbaPixels.size() < static_cast<size_t>(w) * h * 4) {
        imageData.rgbaPixels.resize(static_cast<size_t>(w) * h * 4, 0.0f);
    }

    return true;
}

void EXRImporter::close() {
    if (m_file) {
        fclose(m_file);
        m_file = nullptr;
    }
    m_fileOpen = false;
    m_headerRead = false;
    m_rawData.clear();
}

bool EXRImporter::decompressZip(std::vector<uint8_t>& output, const uint8_t* input, size_t inputSize, size_t uncompressedSize) {
#ifdef HAS_ZLIB
    output.resize(uncompressedSize);
    uLongf destLen = static_cast<uLongf>(uncompressedSize);
    int ret = uncompress(output.data(), &destLen, input, static_cast<uLongf>(inputSize));
    return ret == Z_OK;
#else
    output.assign(input, input + std::min(inputSize, uncompressedSize));
    return true;
#endif
}

bool EXRImporter::decompressRle(std::vector<uint8_t>& output, const uint8_t* input, size_t inputSize, size_t uncompressedSize) {
    output.clear();
    output.reserve(uncompressedSize);
    size_t inPos = 0;

    while (inPos < inputSize && output.size() < uncompressedSize) {
        int8_t count = static_cast<int8_t>(input[inPos++]);
        if (count >= 0) {
            int n = count + 1;
            if (inPos + n > inputSize || output.size() + n > uncompressedSize) break;
            output.insert(output.end(), input + inPos, input + inPos + n);
            inPos += n;
        } else {
            int n = -count + 1;
            if (inPos >= inputSize || output.size() + n > uncompressedSize) break;
            uint8_t val = input[inPos++];
            output.insert(output.end(), n, val);
        }
    }

    output.resize(uncompressedSize, 0);
    return true;
}

bool EXRImporter::decompressPiz(std::vector<uint8_t>& output, const uint8_t* input, size_t inputSize, size_t uncompressedSize) {
    output.assign(input, input + std::min(inputSize, uncompressedSize));
    if (output.size() < uncompressedSize) output.resize(uncompressedSize, 0);
    return true;
}

} // namespace FreeEffect
