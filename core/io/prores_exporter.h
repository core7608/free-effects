#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <functional>

namespace FreeEffect {

enum class ProResProfile {
    Proxy,
    LT,
    Standard,
    HQ,
    Four444,
    Four444XQ
};

struct ProResExportSettings {
    std::string outputPath;
    int width = 1920;
    int height = 1080;
    double frameRate = 30.0;
    ProResProfile profile = ProResProfile::Standard;
    bool interlaced = false;
    bool alphaChannel = false;
    int colorSpace = 1;
    int chromaSubsampling = 0;
};

class ProResExporter {
public:
    ProResExporter() = default;
    ~ProResExporter() = default;

    bool beginExport(const ProResExportSettings& settings);
    bool writeFrame(const uint8_t* rgbaPixels, int width, int height);
    bool endExport();

    bool isOpen() const { return m_open; }
    std::string getErrorMessage() const { return m_errorMessage; }

    static int getBitrateForProfile(ProResProfile profile, int width, int height, double frameRate);

private:
    bool writeMOVHeader();
    bool writeMOVAudioTrack();
    bool writeMOVMetaData();
    bool writeMOVDATAAtom(const std::vector<uint8_t>& frameData);
    bool writeMOVMdatAtom();

    uint32_t calculateChecksum(const uint8_t* data, size_t size);

    bool m_open = false;
    ProResExportSettings m_settings;
    std::string m_errorMessage;
    FILE* m_file = nullptr;
    std::vector<std::vector<uint8_t>> m_frames;
    std::vector<uint32_t> m_frameOffsets;
    uint32_t m_currentOffset = 0;
};

} // namespace FreeEffect
