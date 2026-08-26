#pragma once

#include "../rendering/renderer.h"
#include "../timeline/composition.h"
#include <string>
#include <vector>
#include <functional>

namespace FreeEffect {

enum class ExportFormat {
    MOV,
    MP4,
    AVI,
    WebM,
    GIF,
    PNGSequence,
    TIFFSequence,
    EXRSequence,
    DPXSequence,
    WAV,
    AIFF
};

enum class VideoCodec {
    H264,
    H265,
    ProRes422,
    ProRes4444,
    DNxHD,
    DNxHR,
    VP9,
    FFV1,
    Uncompressed
};

enum class AudioCodec {
    PCM,
    AAC,
    MP3,
    FLAC,
    Vorbis
};

enum class AlphaChannelMode {
    None,
    Straight,
    Premultiplied,
    PremultipliedColor // premultiplied with specific background color
};

enum class FieldRendering {
    None,
    UpperFirst,
    LowerFirst
};

enum class ColorDepth {
    bpc8,
    bpc16,
    bpc32
};

struct ExportSettings {
    std::string outputPath;
    int width = 1920;
    int height = 1080;
    double frameRate = 30.0;
    double sourceFrameRate = 30.0;
    int quality = 100;
    bool exportAudio = true;

    ExportFormat format = ExportFormat::MP4;
    VideoCodec videoCodec = VideoCodec::H264;
    AudioCodec audioCodec = AudioCodec::AAC;
    AlphaChannelMode alphaMode = AlphaChannelMode::None;
    FieldRendering fieldRendering = FieldRendering::None;
    ColorDepth colorDepth = ColorDepth::bpc8;

    bool useProxy = false;
    int proxyScale = 50; // percentage

    // For frame rate conversion
    bool doFrameRateConversion = false;
    double targetFrameRate = 30.0;

    // Legacy string-based codec (kept for compatibility)
    std::string codec = "h264";
};

struct ExportProgressInfo {
    int currentFrame = 0;
    int totalFrames = 0;
    double progress = 0.0;
    std::string currentPass;
    bool isEncoding = false;
};

class Exporter {
public:
    Exporter();
    ~Exporter();

    bool beginExport(const Composition& comp, const ExportSettings& settings);
    bool exportFrame(const PixelBuffer& frame);
    bool endExport();

    bool isExporting() const { return m_exporting; }
    double getProgress() const { return m_progress; }
    ExportProgressInfo getProgressInfo() const { return m_progressInfo; }

    void setCancelRequested(bool cancel) { m_cancelRequested = cancel; }
    bool isCancelRequested() const { return m_cancelRequested; }

    // Image sequence export
    bool exportImageSequence(const Composition& comp, const ExportSettings& settings,
                             const std::string& framePattern);

    // Audio export
    bool exportAudio(const std::vector<float>& samples, int sampleRate, int channels,
                     const ExportSettings& settings);

    // GIF export
    bool beginGIFExport(int width, int height, int frameDelay);
    bool exportGIFFrame(const PixelBuffer& frame);
    bool endGIFExport();

    // Frame rate conversion
    bool needsFrameRateConversion() const;
    double getConvertedTime(double originalTime) const;

    // Proxy helpers
    PixelBuffer createProxyBuffer(const PixelBuffer& fullRes) const;

    // Callback for progress reporting
    void setProgressCallback(std::function<void(const ExportProgressInfo&)> callback) {
        m_progressCallback = callback;
    }

private:
    bool m_exporting = false;
    bool m_cancelRequested = false;
    double m_progress = 0.0;
    ExportSettings m_settings;
    ExportProgressInfo m_progressInfo;
    int m_frameCount = 0;
    std::function<void(const ExportProgressInfo&)> m_progressCallback;

#ifdef HAS_FFMPEG
    struct FFmpegContext;
    FFmpegContext* m_context = nullptr;
#endif

    void reportProgress();
    bool writeImageFrame(const PixelBuffer& frame, const std::string& path);
    std::string getFormatExtension() const;
    std::string getCodecName() const;
};

} // namespace FreeEffect
