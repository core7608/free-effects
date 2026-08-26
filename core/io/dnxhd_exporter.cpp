#include "dnxhd_exporter.h"
#include <cstring>
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static void writeU16BE(FILE* f, uint16_t val) {
    uint8_t b[2] = {static_cast<uint8_t>((val >> 8) & 0xFF), static_cast<uint8_t>(val & 0xFF)};
    fwrite(b, 1, 2, f);
}

static void writeU32BE(FILE* f, uint32_t val) {
    uint8_t b[4] = {
        static_cast<uint8_t>((val >> 24) & 0xFF),
        static_cast<uint8_t>((val >> 16) & 0xFF),
        static_cast<uint8_t>((val >> 8) & 0xFF),
        static_cast<uint8_t>(val & 0xFF)
    };
    fwrite(b, 1, 4, f);
}

int DNXExporter::getBitrateForProfile(DNXProfile profile, int width, int height) {
    int pixels = width * height;
    switch (profile) {
        case DNXProfile::DNxHD_220x: return pixels * 220 * 2 / 1000000;
        case DNXProfile::DNxHD_220: return pixels * 220 * 2 / 1000000;
        case DNXProfile::DNxHD_185x: return pixels * 185 * 2 / 1000000;
        case DNXProfile::DNxHD_185: return pixels * 185 * 2 / 1000000;
        case DNXProfile::DNxHD_120: return pixels * 120 * 2 / 1000000;
        case DNXProfile::DNxHD_45: return pixels * 45 * 2 / 1000000;
        case DNXProfile::DNxHR_HQ: return pixels * 180 * 2 / 1000000;
        case DNXProfile::DNxHR_SQ: return pixels * 100 * 2 / 1000000;
        case DNXProfile::DNxHR_LB: return pixels * 50 * 2 / 1000000;
        case DNXProfile::DNxHR_HQX: return pixels * 200 * 2 / 1000000;
        case DNXProfile::DNxHR_444: return pixels * 400 * 2 / 1000000;
        default: return pixels * 180 * 2 / 1000000;
    }
}

bool DNXExporter::beginExport(const DNXExportSettings& settings) {
    if (m_open) {
        m_errorMessage = "Export already in progress";
        return false;
    }

    m_settings = settings;
    m_frames.clear();
    m_currentOffset = 0;

    m_file = fopen(settings.outputPath.c_str(), "wb");
    if (!m_file) {
        m_errorMessage = "Cannot create output file: " + settings.outputPath;
        return false;
    }

    m_open = true;
    return true;
}

bool DNXExporter::writeFrame(const uint8_t* yuvPixels, int width, int height) {
    if (!m_open || !m_file) {
        m_errorMessage = "Export not open";
        return false;
    }

    std::vector<uint8_t> frameData;
    frameData.resize(width * height * 2);
    std::memcpy(frameData.data(), yuvPixels, frameData.size());

    m_frames.push_back(std::move(frameData));
    m_currentOffset += static_cast<uint32_t>(m_frames.back().size());

    return true;
}

bool DNXExporter::endExport() {
    if (!m_open || !m_file) return false;

    writeMXFHeader();
    writeMXFMetadata();
    writeMXFEssence();
    writeMXFFooter();

    fclose(m_file);
    m_file = nullptr;
    m_open = false;
    m_frames.clear();

    return true;
}

bool DNXExporter::writeMXFHeader() {
    if (!m_file) return false;

    uint8_t headerPartition[16] = {
        0x06, 0x0E, 0x2B, 0x34, 0x02, 0x05, 0x01, 0x01,
        0x0D, 0x01, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01
    };
    fwrite(headerPartition, 1, 16, m_file);

    uint32_t partitionSize = 16 * 100;
    writeU32BE(m_file, partitionSize);
    writeU32BE(m_file, 0);
    writeU32BE(m_file, 0);
    writeU32BE(m_file, 0);

    return true;
}

bool DNXExporter::writeMXFMetadata() {
    return true;
}

bool DNXExporter::writeMXFEssence() {
    if (!m_file) return false;

    for (const auto& frame : m_frames) {
        fwrite(frame.data(), 1, frame.size(), m_file);
    }

    return true;
}

bool DNXExporter::writeMXFFooter() {
    if (!m_file) return false;

    uint8_t footerPartition[16] = {
        0x06, 0x0E, 0x2B, 0x34, 0x02, 0x05, 0x01, 0x01,
        0x0D, 0x01, 0x02, 0x01, 0x01, 0x01, 0x02, 0x01
    };
    fwrite(footerPartition, 1, 16, m_file);
    writeU32BE(m_file, 16 + 16);
    writeU32BE(m_file, 0);
    writeU32BE(m_file, 0);
    writeU32BE(m_file, 0);

    return true;
}

void DNXExporter::convertRGBToYUV422(const uint8_t* rgb, std::vector<uint8_t>& yuv, int width, int height) {
    yuv.resize(static_cast<size_t>(width) * height * 2);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; x += 2) {
            for (int i = 0; i < 2; ++i) {
                int px = x + i;
                if (px >= width) break;
                size_t srcIdx = (static_cast<size_t>(y) * width + px) * 3;
                uint8_t r = rgb[srcIdx + 0];
                uint8_t g = rgb[srcIdx + 1];
                uint8_t b = rgb[srcIdx + 2];

                uint8_t yVal = static_cast<uint8_t>(0.299 * r + 0.587 * g + 0.114 * b);
                uint8_t cbVal = static_cast<uint8_t>(-0.169 * r - 0.331 * g + 0.500 * b + 128);
                uint8_t crVal = static_cast<uint8_t>(0.500 * r - 0.419 * g - 0.081 * b + 128);

                size_t dstIdx = (static_cast<size_t>(y) * width + x) * 2;
                if (i == 0) {
                    yuv[dstIdx + 0] = yVal;
                    yuv[dstIdx + 1] = cbVal;
                } else {
                    yuv[dstIdx + 2] = yVal;
                    yuv[dstIdx + 3] = crVal;
                }
            }
        }
    }
}

} // namespace FreeEffect
