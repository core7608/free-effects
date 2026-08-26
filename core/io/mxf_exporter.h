#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace FreeEffect {

enum class MXFContentType {
    VideoOnly,
    AudioOnly,
    VideoAndAudio
};

enum class MXFVideoCodec {
    MPEG2,
    DNxHD,
    DNxHR,
    ProRes,
    Uncompressed,
    H264,
    JPEG2000
};

struct MXFExportSettings {
    std::string outputPath;
    int width = 1920;
    int height = 1080;
    double frameRate = 29.97;
    MXFVideoCodec videoCodec = MXFVideoCodec::MPEG2;
    MXFContentType contentType = MXFContentType::VideoOnly;
    int audioBitDepth = 24;
    int audioSampleRate = 48000;
    int audioChannels = 2;
    bool interlaced = false;
    std::string materialPackageName;
    std::string sourcePackageName;
};

class MXFExporter {
public:
    MXFExporter() = default;
    ~MXFExporter() = default;

    bool beginExport(const MXFExportSettings& settings);
    bool writeVideoFrame(const uint8_t* frameData, size_t dataSize);
    bool writeAudioSamples(const uint8_t* audioData, size_t dataSize);
    bool endExport();

    bool isOpen() const { return m_open; }
    std::string getErrorMessage() const { return m_errorMessage; }

private:
    bool writeHeaderPartition();
    bool writeMetadata();
    bool writePrimerPack();
    bool writeContentStorage();
    bool writeTrackInfomation();
    bool writeEssenceDescriptors();
    bool writeFooterPartition();

    void writeBER(FILE* f, uint64_t value);
    void writeKLV(FILE* f, const uint8_t* key, uint64_t length, const uint8_t* value);

    bool m_open = false;
    MXFExportSettings m_settings;
    std::string m_errorMessage;
    FILE* m_file = nullptr;
    std::vector<std::vector<uint8_t>> m_videoFrames;
    std::vector<std::vector<uint8_t>> m_audioFrames;
    uint64_t m_totalEssenceSize = 0;
};

} // namespace FreeEffect
