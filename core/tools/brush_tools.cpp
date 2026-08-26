#include "brush_tools.h"
#include <cmath>

namespace FreeEffect {

void BrushTool::onMouseDown(double x, double y, int modifiers) {
    m_drawing = true;
    beginStroke(x, y, 1.0);
}

void BrushTool::onMouseMove(double x, double y, int modifiers) {
    if (!m_drawing) return;
    double speed = 0.0;
    if (!m_currentStroke.points.empty()) {
        const auto& last = m_currentStroke.points.back().position;
        double dx = x - last.x;
        double dy = y - last.y;
        speed = std::sqrt(dx * dx + dy * dy);
    }
    double pressure = std::clamp(1.0 - speed / 200.0, 0.2, 1.0);
    addStrokePoint(x, y, pressure);
}

void BrushTool::onMouseUp(double x, double y, int modifiers) {
    if (!m_drawing) return;
    m_drawing = false;
    endStroke();
}

void BrushTool::beginStroke(double x, double y, double pressure) {
    m_currentStroke = PaintStroke();
    m_currentStroke.brushSize = m_brushSize;
    m_currentStroke.opacity = m_opacity;
    m_currentStroke.flow = m_flow;
    m_currentStroke.colorR = m_colorR;
    m_currentStroke.colorG = m_colorG;
    m_currentStroke.colorB = m_colorB;
    m_currentStroke.frame = m_currentFrame;
    m_currentStroke.points.push_back({{x, y}, pressure, 0.0});
}

void BrushTool::addStrokePoint(double x, double y, double pressure) {
    m_currentStroke.points.push_back({{x, y}, pressure, 0.0});
}

void BrushTool::endStroke() {
    if (!m_currentStroke.points.empty()) {
        m_strokes.push_back(m_currentStroke);
    }
    m_currentStroke = PaintStroke();
}

ToolResult BrushTool::getResult() const {
    ToolResult result;
    result.consumed = m_drawing || !m_strokes.empty();
    result.finished = !m_drawing && !m_strokes.empty();
    for (const auto& pt : m_currentStroke.points) {
        result.points.push_back(pt.position);
    }
    return result;
}

void BrushTool::reset() {
    m_drawing = false;
    m_strokes.clear();
    m_currentStroke = PaintStroke();
}

void CloneStampTool::onMouseDown(double x, double y, int modifiers) {
    if (modifiers & 0x04000000) {
        m_sourcePoint = {x, y};
        m_cloneOffset = {0, 0};
        return;
    }
    if (m_currentStroke.points.empty()) {
        m_cloneOffset = {m_sourcePoint.x - x, m_sourcePoint.y - y};
    }
    BrushTool::onMouseDown(x, y, modifiers);
}

void CloneStampTool::onMouseMove(double x, double y, int modifiers) {
    BrushTool::onMouseMove(x, y, modifiers);
}

void EraserTool::onMouseDown(double x, double y, int modifiers) {
    m_drawing = true;
    beginStroke(x, y, 1.0);
}

void EraserTool::onMouseMove(double x, double y, int modifiers) {
    if (!m_drawing) return;
    addStrokePoint(x, y, 1.0);
}

void EraserTool::onMouseUp(double x, double y, int modifiers) {
    if (!m_drawing) return;
    m_drawing = false;
    endStroke();
}

} // namespace FreeEffect
