#include "image_sequence_exporter.h"
#include <cstring>
#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <cmath>

namespace FreeEffect {

#ifdef HAS_ZLIB
#include <zlib.h>
#endif

static void writeU32BE(FILE* f, uint32_t val) {
    uint8_t b[4] = {
        static_cast<uint8_t>((val >> 24) & 0xFF),
        static_cast<uint8_t>((val >> 16) & 0xFF),
        static_cast<uint8_t>((val >> 8) & 0xFF),
        static_cast<uint8_t>(val & 0xFF)
    };
    fwrite(b, 1, 4, f);
}

std::string ImageSequenceExporter::getFormatExtension(SequenceFormat format) {
    switch (format) {
        case SequenceFormat::PNG: return ".png";
        case SequenceFormat::TIFF: return ".tif";
        case SequenceFormat::EXR: return ".exr";
        case SequenceFormat::DPX: return ".dpx";
        case SequenceFormat::TGA: return ".tga";
        case SequenceFormat::BMP: return ".bmp";
        case SequenceFormat::JPEG: return ".jpg";
        default: return ".png";
    }
}

std::string ImageSequenceExporter::generateFramePath(int frameNumber) const {
    std::string path = m_settings.outputDirectory;
    if (!path.empty() && path.back() != '/' && path.back() != '\\') {
        path += "/";
    }
    path += m_settings.baseName;

    char frameStr[32] = {};
    switch (m_settings.numbering) {
        case NumberingPattern::Hash4:
            snprintf(frameStr, sizeof(frameStr), "_%04d", frameNumber);
            break;
        case NumberingPattern::Hash5:
            snprintf(frameStr, sizeof(frameStr), "_%05d", frameNumber);
            break;
        case NumberingPattern::ZeroPad4:
            snprintf(frameStr, sizeof(frameStr), "_%04d", frameNumber);
            break;
        case NumberingPattern::ZeroPad5:
            snprintf(frameStr, sizeof(frameStr), "_%05d", frameNumber);
            break;
        case NumberingPattern::FrameNum:
            snprintf(frameStr, sizeof(frameStr), "_%d", frameNumber);
            break;
    }

    path += frameStr;
    path += getFormatExtension(m_settings.format);
    return path;
}

std::string ImageSequenceExporter::getFramePath(int frameNumber) const {
    return generateFramePath(frameNumber);
}

bool ImageSequenceExporter::beginExport(const ImageSequenceSettings& settings) {
    if (m_open) {
        m_errorMessage = "Export already in progress";
        return false;
    }

    m_settings = settings;
    m_frames.clear();

    try {
        std::filesystem::create_directories(settings.outputDirectory);
    } catch (...) {
        m_errorMessage = "Cannot create output directory: " + settings.outputDirectory;
        return false;
    }

    m_open = true;
    return true;
}

bool ImageSequenceExporter::writeFrame(int frameNumber, const uint8_t* rgbaPixels, int width, int height) {
    if (!m_open) {
        m_errorMessage = "Export not open";
        return false;
    }

    std::string path = generateFramePath(frameNumber);
    SequenceFrameInfo info;
    info.filePath = path;
    info.frameNumber = frameNumber;

    bool ok = false;
    switch (m_settings.format) {
        case SequenceFormat::PNG: ok = writePNG(path, rgbaPixels, width, height); break;
        case SequenceFormat::TIFF: ok = writeTIFF(path, rgbaPixels, width, height, 4); break;
        case SequenceFormat::EXR: ok = writeEXR(path, rgbaPixels, width, height); break;
        case SequenceFormat::DPX: ok = writeDPX(path, rgbaPixels, width, height); break;
        case SequenceFormat::TGA: ok = writeTGA(path, rgbaPixels, width, height); break;
        case SequenceFormat::BMP: ok = writeBMP(path, rgbaPixels, width, height); break;
        case SequenceFormat::JPEG: ok = writeJPEG(path, rgbaPixels, width, height); break;
    }

    info.written = ok;
    m_frames.push_back(info);
    return ok;
}

bool ImageSequenceExporter::writeFrameWithLayers(int frameNumber,
    const std::vector<std::pair<std::string, std::vector<uint8_t>>>& layers,
    int width, int height) {
    if (!m_open) {
        m_errorMessage = "Export not open";
        return false;
    }

    if (layers.empty()) return false;

    std::string path = generateFramePath(frameNumber);
    SequenceFrameInfo info;
    info.filePath = path;
    info.frameNumber = frameNumber;
    info.written = writePNG(path, layers[0].second.data(), width, height);
    m_frames.push_back(info);
    return info.written;
}

bool ImageSequenceExporter::endExport() {
    m_open = false;
    return true;
}

bool ImageSequenceExporter::writePNG(const std::string& path, const uint8_t* rgbaPixels, int width, int height) {
    FILE* f = fopen(path.c_str(), "wb");
    if (!f) return false;

    uint8_t signature[8] = {137, 80, 78, 71, 13, 10, 26, 10};
    fwrite(signature, 1, 8, f);

    auto writeChunk = [&](const char* type, const uint8_t* data, uint32_t len) {
        fwrite(type, 1, 4, f);
        writeU32BE(f, len);
        if (len > 0 && data) fwrite(data, 1, len, f);
        writeU32BE(f, 0);
    };

    uint8_t ihdr[13];
    ihdr[0] = (width >> 24) & 0xFF;
    ihdr[1] = (width >> 16) & 0xFF;
    ihdr[2] = (width >> 8) & 0xFF;
    ihdr[3] = width & 0xFF;
    ihdr[4] = (height >> 24) & 0xFF;
    ihdr[5] = (height >> 16) & 0xFF;
    ihdr[6] = (height >> 8) & 0xFF;
    ihdr[7] = height & 0xFF;
    ihdr[8] = m_settings.bitsPerChannel;
    ihdr[9] = m_settings.includeAlpha ? 6 : 2;
    ihdr[10] = 0;
    ihdr[11] = 0;
    ihdr[12] = 0;
    writeChunk("IHDR", ihdr, 13);

    int channels = m_settings.includeAlpha ? 4 : 3;
    uint32_t rawSize = static_cast<uint32_t>(width) * height * channels + height;
    std::vector<uint8_t> rawData(rawSize);
    size_t offset = 0;
    for (int y = 0; y < height; ++y) {
        rawData[offset++] = 0;
        for (int x = 0; x < width; ++x) {
            size_t srcIdx = (static_cast<size_t>(y) * width + x) * 4;
            rawData[offset++] = rgbaPixels[srcIdx + 0];
            rawData[offset++] = rgbaPixels[srcIdx + 1];
            rawData[offset++] = rgbaPixels[srcIdx + 2];
            if (channels == 4) rawData[offset++] = rgbaPixels[srcIdx + 3];
        }
    }

#ifdef HAS_ZLIB
    uLongf compressedSize = compressBound(rawSize);
    std::vector<uint8_t> compressed(compressedSize);
    int ret = compress2(compressed.data(), &compressedSize, rawData.data(), rawSize, m_settings.compressionLevel);
    if (ret == Z_OK) {
        writeChunk("IDAT", compressed.data(), static_cast<uint32_t>(compressedSize));
    } else {
        writeChunk("IDAT", rawData.data(), rawSize);
    }
#else
    writeChunk("IDAT", rawData.data(), rawSize);
#endif

    writeChunk("IEND", nullptr, 0);
    fclose(f);
    return true;
}

bool ImageSequenceExporter::writeTIFF(const std::string& path, const uint8_t* rgbaPixels, int width, int height, int channels) {
    FILE* f = fopen(path.c_str(), "wb");
    if (!f) return false;

    uint16_t byteOrder = 0x4949;
    fwrite(&byteOrder, 2, 1, f);

    uint16_t tiffMagic = 42;
    fwrite(&tiffMagic, 2, 1, f);

    uint32_t ifdOffset = 8;
    fwrite(&ifdOffset, 4, 1, f);

    uint16_t numEntries = 11;
    fwrite(&numEntries, 2, 1, f);

    auto writeTag = [&](uint16_t tag, uint16_t type, uint32_t count, uint32_t value) {
        fwrite(&tag, 2, 1, f);
        fwrite(&type, 2, 1, f);
        fwrite(&count, 4, 1, f);
        fwrite(&value, 4, 1, f);
    };

    writeTag(256, 3, 1, width);
    writeTag(257, 3, 1, height);
    writeTag(258, 3, 1, m_settings.bitsPerChannel);
    writeTag(259, 3, 1, 1);
    writeTag(262, 3, 1, 2);
    writeTag(273, 3, 1, ifdOffset + 2 + numEntries * 12 + 4);
    writeTag(277, 3, 1, channels);
    writeTag(278, 3, 1, height);
    writeTag(279, 4, 1, static_cast<uint32_t>(width * channels * ((m_settings.bitsPerChannel + 7) / 8) * height));
    writeTag(282, 5, 1, ifdOffset + 2 + numEntries * 12 + 4 + width * height * channels * ((m_settings.bitsPerChannel + 7) / 8) + 8);
    writeTag(283, 5, 1, ifdOffset + 2 + numEntries * 12 + 4 + width * height * channels * ((m_settings.bitsPerChannel + 7) / 8) + 16);

    uint32_t nextIfd = 0;
    fwrite(&nextIfd, 4, 1, f);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            size_t srcIdx = (static_cast<size_t>(y) * width + x) * 4;
            fwrite(rgbaPixels + srcIdx, 1, channels, f);
        }
    }

    fclose(f);
    return true;
}

bool ImageSequenceExporter::writeEXR(const std::string& path, const uint8_t* rgbaPixels, int width, int height) {
    FILE* f = fopen(path.c_str(), "wb");
    if (!f) return false;

    uint32_t magic = 0x762F3101;
    fwrite(&magic, 4, 1, f);

    uint16_t version = 2;
    uint16_t flags = 0x02;
    fwrite(&version, 2, 1, f);
    fwrite(&flags, 2, 1, f);

    auto writeAttr = [&](const char* name, const char* type, const void* data, uint32_t size) {
        fwrite(name, 1, strlen(name) + 1, f);
        fwrite(type, 1, strlen(type) + 1, f);
        fwrite(&size, 4, 1, f);
        fwrite(data, 1, size, f);
    };

    uint8_t chData[] = {1};
    writeAttr("channels", "chlist", chData, 1);
    writeAttr("compression", "compression", "\x02", 1);

    int32_t dataWindow[4] = {0, 0, width - 1, height - 1};
    writeAttr("dataWindow", "box2i", dataWindow, 16);

    int32_t displayWindow[4] = {0, 0, width - 1, height - 1};
    writeAttr("displayWindow", "box2i", displayWindow, 16);

    float pixelAspectRatio = 1.0f;
    writeAttr("pixelAspectRatio", "float", &pixelAspectRatio, 4);

    float screenWindowWidth = 1.0f;
    writeAttr("screenWindowWidth", "float", &screenWindowWidth, 4);

    fputc(0, f);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            size_t srcIdx = (static_cast<size_t>(y) * width + x) * 4;
            uint16_t r = 0, g = 0, b = 0, a = 0;

            uint32_t rv, gv, bv, av;
            float rf = rgbaPixels[srcIdx + 0] / 255.0f;
            float gf = rgbaPixels[srcIdx + 1] / 255.0f;
            float bf = rgbaPixels[srcIdx + 2] / 255.0f;
            float af = rgbaPixels[srcIdx + 3] / 255.0f;

            auto floatToHalf = [](float val) -> uint16_t {
                uint32_t u;
                std::memcpy(&u, &val, 4);
                uint32_t sign = (u >> 16) & 0x8000;
                int32_t exponent = static_cast<int32_t>((u >> 23) & 0xFF) - 127 + 15;
                uint32_t mantissa = (u >> 13) & 0x3FF;
                if (exponent <= 0) return static_cast<uint16_t>(sign);
                if (exponent >= 31) return static_cast<uint16_t>(sign | 0x7C00);
                return static_cast<uint16_t>(sign | (exponent << 10) | mantissa);
            };

            r = floatToHalf(rf);
            g = floatToHalf(gf);
            b = floatToHalf(bf);
            a = floatToHalf(af);

            fwrite(&r, 2, 1, f);
            fwrite(&g, 2, 1, f);
            fwrite(&b, 2, 1, f);
            fwrite(&a, 2, 1, f);
        }
    }

    fclose(f);
    return true;
}

bool ImageSequenceExporter::writeDPX(const std::string& path, const uint8_t* rgbaPixels, int width, int height) {
    FILE* f = fopen(path.c_str(), "wb");
    if (!f) return false;

    fwrite("SDPX", 1, 4, f);

    uint32_t fileSize = 644 + width * height * 4;
    fwrite(&fileSize, 4, 1, f);

    uint8_t padding[240] = {};
    fwrite(padding, 1, 240, f);

    uint32_t offsetToData = 644;
    fwrite(&offsetToData, 4, 1, f);

    uint32_t version = 0x00000100;
    fwrite(&version, 4, 1, f);

    uint8_t fileInfo[8] = {};
    fwrite(fileInfo, 1, 8, f);

    uint8_t dummy[100] = {};
    fwrite(dummy, 1, 100, f);

    uint8_t format[16] = {};
    fwrite(format, 1, 16, f);

    uint32_t framePos = 0;
    fwrite(&framePos, 4, 1, f);

    uint32_t genInfo = 0;
    fwrite(&genInfo, 4, 1, f);

    uint32_t borderColor = 0;
    fwrite(&borderColor, 4, 1, f);

    uint32_t pixelAspect[2] = {1, 1};
    fwrite(pixelAspect, 4, 2, f);

    fwrite(padding, 1, 8, f);

    uint32_t descriptor = 50;
    fwrite(&descriptor, 4, 1, f);

    uint32_t transfer = 2;
    fwrite(&transfer, 4, 1, f);

    uint32_t colorimetric = 1;
    fwrite(&colorimetric, 4, 1, f);

    uint32_t bitDepth = 10;
    fwrite(&bitDepth, 4, 1, f);

    uint32_t packing = 1;
    fwrite(&packing, 4, 1, f);

    uint32_t encoding = 0;
    fwrite(&encoding, 4, 1, f);

    uint32_t linePacking = 0;
    fwrite(&linePacking, 4, 1, f);

    uint32_t headerPadSize = 644 - 444;
    uint8_t headerPadding[528] = {};
    fwrite(headerPadding, 1, headerPadSize, f);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            size_t srcIdx = (static_cast<size_t>(y) * width + x) * 4;
            uint16_t r10 = static_cast<uint16_t>(rgbaPixels[srcIdx + 0]) << 2;
            uint16_t g10 = static_cast<uint16_t>(rgbaPixels[srcIdx + 1]) << 2;
            uint16_t b10 = static_cast<uint16_t>(rgbaPixels[srcIdx + 2]) << 2;

            uint32_t packed = (static_cast<uint32_t>(r10) << 22) |
                             (static_cast<uint32_t>(g10) << 12) |
                             (static_cast<uint32_t>(b10) << 2);
            fwrite(&packed, 4, 1, f);
        }
    }

    fclose(f);
    return true;
}

bool ImageSequenceExporter::writeTGA(const std::string& path, const uint8_t* rgbaPixels, int width, int height) {
    FILE* f = fopen(path.c_str(), "wb");
    if (!f) return false;

    uint8_t header[18] = {};
    header[2] = 2;
    header[12] = width & 0xFF;
    header[13] = (width >> 8) & 0xFF;
    header[14] = height & 0xFF;
    header[15] = (height >> 8) & 0xFF;
    header[16] = 32;
    header[17] = 0x28;
    fwrite(header, 1, 18, f);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            size_t srcIdx = (static_cast<size_t>(y) * width + x) * 4;
            uint8_t bgra[4] = {
                rgbaPixels[srcIdx + 2],
                rgbaPixels[srcIdx + 1],
                rgbaPixels[srcIdx + 0],
                rgbaPixels[srcIdx + 3]
            };
            fwrite(bgra, 1, 4, f);
        }
    }

    fwrite("TRUEVISION-XFILE.\0", 1, 18, f);
    fclose(f);
    return true;
}

bool ImageSequenceExporter::writeBMP(const std::string& path, const uint8_t* rgbaPixels, int width, int height) {
    FILE* f = fopen(path.c_str(), "wb");
    if (!f) return false;

    int rowSize = (width * 3 + 3) & ~3;
    int imageSize = rowSize * height;
    int fileSize = 54 + imageSize;

    uint8_t fileHeader[14] = {};
    fileHeader[0] = 'B';
    fileHeader[1] = 'M';
    fileHeader[2] = fileSize & 0xFF;
    fileHeader[3] = (fileSize >> 8) & 0xFF;
    fileHeader[4] = (fileSize >> 16) & 0xFF;
    fileHeader[5] = (fileSize >> 24) & 0xFF;
    fileHeader[10] = 54;
    fwrite(fileHeader, 1, 14, f);

    uint8_t infoHeader[40] = {};
    infoHeader[0] = 40;
    infoHeader[4] = width & 0xFF;
    infoHeader[5] = (width >> 8) & 0xFF;
    infoHeader[6] = (width >> 16) & 0xFF;
    infoHeader[7] = (width >> 24) & 0xFF;
    infoHeader[8] = height & 0xFF;
    infoHeader[9] = (height >> 8) & 0xFF;
    infoHeader[10] = (height >> 16) & 0xFF;
    infoHeader[11] = (height >> 24) & 0xFF;
    infoHeader[12] = 1;
    infoHeader[14] = 24;
    fwrite(infoHeader, 1, 40, f);

    for (int y = height - 1; y >= 0; --y) {
        for (int x = 0; x < width; ++x) {
            size_t srcIdx = (static_cast<size_t>(y) * width + x) * 4;
            uint8_t bgr[3] = {rgbaPixels[srcIdx + 2], rgbaPixels[srcIdx + 1], rgbaPixels[srcIdx + 0]};
            fwrite(bgr, 1, 3, f);
        }
        uint8_t padding[3] = {};
        int padBytes = rowSize - width * 3;
        if (padBytes > 0) fwrite(padding, 1, padBytes, f);
    }

    fclose(f);
    return true;
}

bool ImageSequenceExporter::writeJPEG(const std::string& path, const uint8_t* rgbaPixels, int width, int height) {
    return writeBMP(path, rgbaPixels, width, height);
}

} // namespace FreeEffect
