#include "prores_exporter.h"
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

static void writeU64BE(FILE* f, uint64_t val) {
    uint8_t b[8];
    for (int i = 7; i >= 0; --i) {
        b[7 - i] = static_cast<uint8_t>(val & 0xFF);
        val >>= 8;
    }
    fwrite(b, 1, 8, f);
}

int ProResExporter::getBitrateForProfile(ProResProfile profile, int width, int height, double frameRate) {
    double pixelsPerFrame = width * height;
    double factor = 0;
    switch (profile) {
        case ProResProfile::Proxy: factor = 0.2; break;
        case ProResProfile::LT: factor = 0.35; break;
        case ProResProfile::Standard: factor = 0.5; break;
        case ProResProfile::HQ: factor = 0.75; break;
        case ProResProfile::Four444: factor = 1.0; break;
        case ProResProfile::Four444XQ: factor = 1.5; break;
    }
    return static_cast<int>(pixelsPerFrame * factor * frameRate * 3 / 8 / 1000);
}

bool ProResExporter::beginExport(const ProResExportSettings& settings) {
    if (m_open) {
        m_errorMessage = "Export already in progress";
        return false;
    }

    m_settings = settings;
    m_frames.clear();
    m_frameOffsets.clear();
    m_currentOffset = 0;

    m_file = fopen(settings.outputPath.c_str(), "wb");
    if (!m_file) {
        m_errorMessage = "Cannot create output file: " + settings.outputPath;
        return false;
    }

    m_open = true;
    return true;
}

bool ProResExporter::writeFrame(const uint8_t* rgbaPixels, int width, int height) {
    if (!m_open || !m_file) {
        m_errorMessage = "Export not open";
        return false;
    }

    std::vector<uint8_t> frameData;
    frameData.resize(width * height * 4);
    std::memcpy(frameData.data(), rgbaPixels, frameData.size());

    if (m_settings.profile == ProResProfile::Four444 || m_settings.profile == ProResProfile::Four444XQ) {
        for (int i = 0; i < width * height; ++i) {
            uint8_t r = frameData[i * 4 + 0];
            uint8_t g = frameData[i * 4 + 1];
            uint8_t b = frameData[i * 4 + 2];
            uint8_t a = frameData[i * 4 + 3];

            int16_t y = static_cast<int16_t>(0.299 * r + 0.587 * g + 0.114 * b);
            int16_t cb = static_cast<int16_t>(-0.169 * r - 0.331 * g + 0.500 * b + 128);
            int16_t cr = static_cast<int16_t>(0.500 * r - 0.419 * g - 0.081 * b + 128);

            frameData[i * 4 + 0] = static_cast<uint8_t>(std::clamp(y, static_cast<int16_t>(0), static_cast<int16_t>(255)));
            frameData[i * 4 + 1] = static_cast<uint8_t>(std::clamp(cb, static_cast<int16_t>(0), static_cast<int16_t>(255)));
            frameData[i * 4 + 2] = static_cast<uint8_t>(std::clamp(cr, static_cast<int16_t>(0), static_cast<int16_t>(255)));
            frameData[i * 4 + 3] = a;
        }
    }

    m_frameOffsets.push_back(m_currentOffset);
    m_frames.push_back(std::move(frameData));
    m_currentOffset += static_cast<uint32_t>(m_frames.back().size());

    return true;
}

bool ProResExporter::endExport() {
    if (!m_open || !m_file) return false;

    writeMOVHeader();
    writeMOVMetaData();

    uint32_t mdatStart = static_cast<uint32_t>(ftell(m_file));
    fwrite("mdat", 1, 4, m_file);
    uint32_t mdatSize = 8;
    for (const auto& frame : m_frames) {
        mdatSize += static_cast<uint32_t>(frame.size());
    }
    writeU32BE(m_file, mdatSize);

    for (const auto& frame : m_frames) {
        fwrite(frame.data(), 1, frame.size(), m_file);
    }

    uint32_t moovPos = static_cast<uint32_t>(ftell(m_file));

    fseek(m_file, mdatStart, SEEK_SET);
    fseek(m_file, 0, SEEK_END);

    fwrite("moov", 1, 4, m_file);
    uint32_t moovSize = 108 + static_cast<uint32_t>(m_frames.size()) * 16;
    writeU32BE(m_file, moovSize);

    fwrite("mvhd", 1, 4, m_file);
    writeU32BE(m_file, 108);
    writeU32BE(m_file, 0);
    writeU32BE(m_file, static_cast<uint32_t>(m_settings.frameRate * 60));
    writeU32BE(m_file, 0);
    writeU16BE(m_file, 0);
    writeU16BE(m_file, 1);
    writeU16BE(m_file, 0);
    for (int i = 0; i < 5; ++i) writeU32BE(m_file, 0x00010000);
    writeU32BE(m_file, 0);
    writeU32BE(m_file, 0);
    for (int i = 0; i < 2; ++i) writeU32BE(m_file, 0x00010000);
    for (int i = 0; i < 6; ++i) writeU32BE(m_file, 0);
    writeU32BE(m_file, 0);
    writeU32BE(m_file, static_cast<uint32_t>(m_frames.size()));
    writeU32BE(m_file, 0);

    fclose(m_file);
    m_file = nullptr;
    m_open = false;
    m_frames.clear();
    m_frameOffsets.clear();

    return true;
}

bool ProResExporter::writeMOVHeader() {
    if (!m_file) return false;
    fseek(m_file, 0, SEEK_SET);

    fwrite("ftyp", 1, 4, m_file);
    writeU32BE(m_file, 20);
    fwrite("qt  ", 1, 4, m_file);
    writeU32BE(m_file, 0x200);
    fwrite("qt  ", 1, 4, m_file);
    fwrite("free", 1, 4, m_file);
    writeU32BE(m_file, 8);
    fwrite("wide", 1, 4, m_file);
    writeU32BE(m_file, 8);

    return true;
}

bool ProResExporter::writeMOVMetaData() {
    if (!m_file) return false;
    return true;
}

bool ProResExporter::writeMOVDATAAtom(const std::vector<uint8_t>& frameData) {
    if (!m_file) return false;
    fwrite(frameData.data(), 1, frameData.size(), m_file);
    return true;
}

bool ProResExporter::writeMOVMdatAtom() {
    return true;
}

uint32_t ProResExporter::calculateChecksum(const uint8_t* data, size_t size) {
    uint32_t sum = 0;
    for (size_t i = 0; i < size; ++i) {
        sum = (sum << 1) ^ data[i];
    }
    return sum;
}

} // namespace FreeEffect
