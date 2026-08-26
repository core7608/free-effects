#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <functional>

namespace FreeEffect {

enum class DNXProfile {
    DNxHD_220x,
    DNxHD_220,
    DNxHD_185x,
    DNxHD_185,
    DNxHD_120,
    DNxHD_45,
    DNxHR_HQ,
    DNxHR_SQ,
    DNxHR_LB,
    DNxHR_HQX,
    DNxHR_444
};

struct DNXExportSettings {
    std::string outputPath;
    int width = 1920;
    int height = 1080;
    double frameRate = 29.97;
    DNXProfile profile = DNXProfile::DNxHD_185;
    bool interlaced = false;
    bool alphaChannel = false;
    int audioBitDepth = 24;
    int audioSampleRate = 48000;
    int audioChannels = 2;
};

class DNXExporter {
public:
    DNXExporter() = default;
    ~DNXExporter() = default;

    bool beginExport(const DNXExportSettings& settings);
    bool writeFrame(const uint8_t* yuvPixels, int width, int height);
    bool endExport();

    bool isOpen() const { return m_open; }
    std::string getErrorMessage() const { return m_errorMessage; }

    static int getBitrateForProfile(DNXProfile profile, int width, int height);

private:
    bool writeMXFHeader();
    bool writeMXFHeaderPartition();
    bool writeMXFMetadata();
    bool writeMXFEssence();
    bool writeMXFFooter();

    void convertRGBToYUV422(const uint8_t* rgb, std::vector<uint8_t>& yuv, int width, int height);

    bool m_open = false;
    DNXExportSettings m_settings;
    std::string m_errorMessage;
    FILE* m_file = nullptr;
    std::vector<std::vector<uint8_t>> m_frames;
    uint32_t m_currentOffset = 0;
};

} // namespace FreeEffect
