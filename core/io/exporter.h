#pragma once

#include "../rendering/renderer.h"
#include "../timeline/composition.h"
#include <string>

namespace FreeEffect {

struct ExportSettings {
    std::string outputPath;
    int width = 1920;
    int height = 1080;
    double frameRate = 30.0;
    int quality = 100;
    bool exportAudio = true;
    std::string codec = "h264";
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
    
    void setCancelRequested(bool cancel) { m_cancelRequested = cancel; }
    bool isCancelRequested() const { return m_cancelRequested; }

private:
    bool m_exporting = false;
    bool m_cancelRequested = false;
    double m_progress = 0.0;
    ExportSettings m_settings;
    
#ifdef HAS_FFMPEG
    struct FFmpegContext;
    FFmpegContext* m_context = nullptr;
#endif
};

} // namespace FreeEffect
