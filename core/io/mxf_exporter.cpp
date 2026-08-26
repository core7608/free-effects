#include "mxf_exporter.h"
#include <cstring>
#include <algorithm>

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

void MXFExporter::writeBER(FILE* f, uint64_t value) {
    if (value < 0x80) {
        uint8_t b = static_cast<uint8_t>(value);
        fwrite(&b, 1, 1, f);
    } else if (value <= 0xFF) {
        uint8_t b[2] = {0x81, static_cast<uint8_t>(value)};
        fwrite(b, 1, 2, f);
    } else if (value <= 0xFFFF) {
        uint8_t b[3] = {0x82, static_cast<uint8_t>((value >> 8) & 0xFF), static_cast<uint8_t>(value & 0xFF)};
        fwrite(b, 1, 3, f);
    } else if (value <= 0xFFFFFF) {
        uint8_t b[4] = {0x83,
            static_cast<uint8_t>((value >> 16) & 0xFF),
            static_cast<uint8_t>((value >> 8) & 0xFF),
            static_cast<uint8_t>(value & 0xFF)
        };
        fwrite(b, 1, 4, f);
    } else {
        uint8_t b[5] = {0x84,
            static_cast<uint8_t>((value >> 24) & 0xFF),
            static_cast<uint8_t>((value >> 16) & 0xFF),
            static_cast<uint8_t>((value >> 8) & 0xFF),
            static_cast<uint8_t>(value & 0xFF)
        };
        fwrite(b, 1, 5, f);
    }
}

void MXFExporter::writeKLV(FILE* f, const uint8_t* key, uint64_t length, const uint8_t* value) {
    fwrite(key, 1, 16, f);
    writeBER(f, length);
    if (length > 0 && value) {
        fwrite(value, 1, static_cast<size_t>(length), f);
    }
}

bool MXFExporter::beginExport(const MXFExportSettings& settings) {
    if (m_open) {
        m_errorMessage = "Export already in progress";
        return false;
    }

    m_settings = settings;
    m_videoFrames.clear();
    m_audioFrames.clear();
    m_totalEssenceSize = 0;

    m_file = fopen(settings.outputPath.c_str(), "wb");
    if (!m_file) {
        m_errorMessage = "Cannot create output file: " + settings.outputPath;
        return false;
    }

    m_open = true;
    return true;
}

bool MXFExporter::writeVideoFrame(const uint8_t* frameData, size_t dataSize) {
    if (!m_open || !m_file) {
        m_errorMessage = "Export not open";
        return false;
    }

    std::vector<uint8_t> frame(frameData, frameData + dataSize);
    m_videoFrames.push_back(std::move(frame));
    m_totalEssenceSize += dataSize;

    return true;
}

bool MXFExporter::writeAudioSamples(const uint8_t* audioData, size_t dataSize) {
    if (!m_open || !m_file) {
        m_errorMessage = "Export not open";
        return false;
    }

    std::vector<uint8_t> samples(audioData, audioData + dataSize);
    m_audioFrames.push_back(std::move(samples));
    m_totalEssenceSize += dataSize;

    return true;
}

bool MXFExporter::endExport() {
    if (!m_open || !m_file) return false;

    writeHeaderPartition();
    writePrimerPack();
    writeContentStorage();
    writeTrackInfomation();
    writeEssenceDescriptors();
    writeFooterPartition();

    fclose(m_file);
    m_file = nullptr;
    m_open = false;
    m_videoFrames.clear();
    m_audioFrames.clear();

    return true;
}

bool MXFExporter::writeHeaderPartition() {
    if (!m_file) return false;

    uint8_t headerKey[16] = {
        0x06, 0x0E, 0x2B, 0x34, 0x02, 0x05, 0x01, 0x01,
        0x0D, 0x01, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01
    };
    fwrite(headerKey, 1, 16, m_file);

    uint64_t totalSize = 100 + m_totalEssenceSize + 200;
    writeBER(m_file, totalSize);

    uint8_t partitionData[24] = {};
    partitionData[0] = 0x7F;
    writeU32BE(m_file, 0);
    writeU32BE(m_file, 0);

    return true;
}

bool MXFExporter::writePrimerPack() {
    return true;
}

bool MXFExporter::writeContentStorage() {
    return true;
}

bool MXFExporter::writeTrackInfomation() {
    return true;
}

bool MXFExporter::writeEssenceDescriptors() {
    if (!m_file) return false;

    for (const auto& frame : m_videoFrames) {
        fwrite(frame.data(), 1, frame.size(), m_file);
    }

    for (const auto& samples : m_audioFrames) {
        fwrite(samples.data(), 1, samples.size(), m_file);
    }

    return true;
}

bool MXFExporter::writeFooterPartition() {
    if (!m_file) return false;

    uint8_t footerKey[16] = {
        0x06, 0x0E, 0x2B, 0x34, 0x02, 0x05, 0x01, 0x01,
        0x0D, 0x01, 0x02, 0x01, 0x01, 0x01, 0x02, 0x01
    };
    fwrite(footerKey, 1, 16, m_file);
    writeBER(m_file, 32);

    return true;
}

} // namespace FreeEffect
