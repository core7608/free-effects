#include "auto_trace_command.h"
#include "../rendering/renderer.h"
#include <cmath>
#include <algorithm>

namespace FreeEffect {

AutoTraceCommand::AutoTraceCommand(
    Composition* comp, int layerIndex, double time,
    TraceMode mode, int threshold, double tolerance, bool invert)
    : m_comp(comp)
    , m_layerIndex(layerIndex)
    , m_time(time)
    , m_mode(mode)
    , m_threshold(threshold)
    , m_tolerance(tolerance)
    , m_invert(invert) {
}

void AutoTraceCommand::execute() {
    const auto& layers = m_comp->getLayers();
    if (m_layerIndex < 0 || m_layerIndex >= static_cast<int>(layers.size())) return;

    auto layer = layers[m_layerIndex];
    if (!layer->isActiveAtTime(m_time)) return;

    int compW = m_comp->getResolution().width;
    int compH = m_comp->getResolution().height;

    Renderer renderer;
    renderer.setQuality(100);
    PixelBuffer buffer = renderer.renderFrame(*m_comp, m_time);

    if (buffer.width <= 0 || buffer.height <= 0) return;

    traceFromBuffer(buffer.width, buffer.height,
                    buffer.data.data(), 4);

    if (!m_tracedPaths.empty()) {
        m_maskLayer = m_comp->addLayer(layer->getName() + " [trace]", LayerType::Shape);
        m_maskLayer->setStartTime(layer->getStartTime());
        m_maskLayer->setDuration(layer->getDuration());
        m_maskLayer->setVisible(true);
    }

    m_executed = true;
}

void AutoTraceCommand::undo() {
    if (m_executed && m_maskLayer) {
        m_comp->removeLayer(m_maskLayer->getId());
        m_maskLayer = nullptr;
    }
    m_tracedPaths.clear();
    m_executed = false;
}

std::string AutoTraceCommand::getDescription() const {
    return "Auto Trace";
}

void AutoTraceCommand::traceFromBuffer(
    int width, int height, const uint8_t* data, int channels) {

    int gridW = width / 4;
    int gridH = height / 4;
    if (gridW < 2 || gridH < 2) return;

    double cellW = static_cast<double>(width) / gridW;
    double cellH = static_cast<double>(height) / gridH;

    std::vector<bool> grid(gridW * gridH);

    for (int gy = 0; gy < gridH; gy++) {
        for (int gx = 0; gx < gridW; gx++) {
            int px = static_cast<int>((gx + 0.5) * cellW);
            int py = static_cast<int>((gy + 0.5) * cellH);
            px = std::clamp(px, 0, width - 1);
            py = std::clamp(py, 0, height - 1);

            int offset = (py * width + px) * channels;
            bool aboveThreshold = false;

            switch (m_mode) {
                case TraceMode::Alpha:
                    if (channels >= 4) {
                        aboveThreshold = data[offset + 3] >= m_threshold;
                    }
                    break;
                case TraceMode::Luminance:
                    if (channels >= 3) {
                        double lum = 0.299 * data[offset] +
                                     0.587 * data[offset + 1] +
                                     0.114 * data[offset + 2];
                        aboveThreshold = lum >= m_threshold;
                    }
                    break;
                case TraceMode::Channels:
                    if (channels >= 3) {
                        aboveThreshold = data[offset] >= m_threshold ||
                                         data[offset + 1] >= m_threshold ||
                                         data[offset + 2] >= m_threshold;
                    }
                    break;
            }

            if (m_invert) aboveThreshold = !aboveThreshold;
            grid[gy * gridW + gx] = aboveThreshold;
        }
    }

    marchingSquares(grid, gridW, gridH, cellW, cellH, 0, 0);
}

void AutoTraceCommand::marchingSquares(
    const std::vector<bool>& grid, int gridW, int gridH,
    double cellW, double cellH, double offsetX, double offsetY) {

    for (int gy = 0; gy < gridH - 1; gy++) {
        for (int gx = 0; gx < gridW - 1; gx++) {
            bool tl = grid[gy * gridW + gx];
            bool tr = grid[gy * gridW + gx + 1];
            bool br = grid[(gy + 1) * gridW + gx + 1];
            bool bl = grid[(gy + 1) * gridW + gx];

            int caseIndex = (tl ? 8 : 0) | (tr ? 4 : 0) | (br ? 2 : 0) | (bl ? 1 : 0);

            if (caseIndex == 0 || caseIndex == 15) continue;

            double cx = offsetX + gx * cellW;
            double cy = offsetY + gy * cellW;

            double topX = cx + cellW * 0.5;
            double topY = cy;
            double rightX = cx + cellW;
            double rightY = cy + cellH * 0.5;
            double bottomX = cx + cellW * 0.5;
            double bottomY = cy + cellH;
            double leftX = cx;
            double leftY = cy + cellH * 0.5;

            ShapePath path;
            path.closed = true;

            switch (caseIndex) {
                case 1: case 14:
                    path.points = {{leftX, leftY}, {bottomX, bottomY}};
                    break;
                case 2: case 13:
                    path.points = {{bottomX, bottomY}, {rightX, rightY}};
                    break;
                case 3: case 12:
                    path.points = {{leftX, leftY}, {rightX, rightY}};
                    break;
                case 4: case 11:
                    path.points = {{topX, topY}, {rightX, rightY}};
                    break;
                case 5:
                    path.points = {{topX, topY}, {rightX, rightY}, {bottomX, bottomY}, {leftX, leftY}};
                    break;
                case 6: case 9:
                    path.points = {{topX, topY}, {bottomX, bottomY}};
                    break;
                case 7: case 8:
                    path.points = {{leftX, leftY}, {topX, topY}};
                    break;
                case 10:
                    path.points = {{topX, topY}, {rightX, rightY}, {bottomX, bottomY}, {leftX, leftY}};
                    break;
                default:
                    break;
            }

            if (path.points.size() >= 2) {
                m_tracedPaths.push_back(path);
            }
        }
    }
}

} // namespace FreeEffect
