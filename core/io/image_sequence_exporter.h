#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <functional>

namespace FreeEffect {

enum class SequenceFormat {
    PNG,
    TIFF,
    EXR,
    DPX,
    TGA,
    BMP,
    JPEG
};

enum class NumberingPattern {
    Hash4,      // file_####.ext
    Hash5,      // file_#####.ext
    ZeroPad4,   // file_%04d.ext
    ZeroPad5,   // file_%05d.ext
    FrameNum    // file_0001.ext
};

struct ImageSequenceSettings {
    std::string outputDirectory;
    std::string baseName;
    SequenceFormat format = SequenceFormat::PNG;
    NumberingPattern numbering = NumberingPattern::Hash4;
    int startFrame = 1;
    int framePadding = 4;
    bool includeAlpha = false;
    int bitsPerChannel = 8;
    int compressionLevel = 6;
    bool multiLayer = false;
    std::vector<std::string> layerNames;
};

struct SequenceFrameInfo {
    std::string filePath;
    int frameNumber = 0;
    bool written = false;
};

class ImageSequenceExporter {
public:
    ImageSequenceExporter() = default;
    ~ImageSequenceExporter() = default;

    bool beginExport(const ImageSequenceSettings& settings);
    bool writeFrame(int frameNumber, const uint8_t* rgbaPixels, int width, int height);
    bool writeFrameWithLayers(int frameNumber, const std::vector<std::pair<std::string, std::vector<uint8_t>>>& layers,
                              int width, int height);
    bool endExport();

    bool isOpen() const { return m_open; }
    std::string getErrorMessage() const { return m_errorMessage; }
    const std::vector<SequenceFrameInfo>& getWrittenFrames() const { return m_frames; }

    std::string getFramePath(int frameNumber) const;
    static std::string getFormatExtension(SequenceFormat format);

private:
    bool writePNG(const std::string& path, const uint8_t* rgbaPixels, int width, int height);
    bool writeTIFF(const std::string& path, const uint8_t* rgbaPixels, int width, int height, int channels);
    bool writeEXR(const std::string& path, const uint8_t* rgbaPixels, int width, int height);
    bool writeDPX(const std::string& path, const uint8_t* rgbaPixels, int width, int height);
    bool writeTGA(const std::string& path, const uint8_t* rgbaPixels, int width, int height);
    bool writeBMP(const std::string& path, const uint8_t* rgbaPixels, int width, int height);
    bool writeJPEG(const std::string& path, const uint8_t* rgbaPixels, int width, int height);

    std::string generateFramePath(int frameNumber) const;

    bool m_open = false;
    ImageSequenceSettings m_settings;
    std::string m_errorMessage;
    std::vector<SequenceFrameInfo> m_frames;
};

} // namespace FreeEffect
