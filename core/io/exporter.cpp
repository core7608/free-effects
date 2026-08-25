#include "exporter.h"

namespace FreeEffect {

Exporter::Exporter() {
}

Exporter::~Exporter() {
    endExport();
}

bool Exporter::beginExport(const Composition& comp, const ExportSettings& settings) {
    if (m_exporting) return false;
    
    m_settings = settings;
    m_progress = 0.0;
    m_cancelRequested = false;
    m_exporting = true;
    
    return true;
}

bool Exporter::exportFrame(const PixelBuffer& frame) {
    if (!m_exporting || m_cancelRequested) return false;
    
    // Stub - FFmpeg encoding will be added later
    return true;
}

bool Exporter::endExport() {
    if (!m_exporting) return true;
    
    m_exporting = false;
    m_progress = 1.0;
    
    return true;
}

} // namespace FreeEffect
