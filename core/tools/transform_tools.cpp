#include "transform_tools.h"
#include <cmath>
#include <algorithm>

namespace FreeEffect {

static constexpr double kPi = 3.14159265358979323846;

void HandTool::onMouseDown(double x, double y, int modifiers) {
    m_startPos = {x, y};
    m_delta = {0, 0};
    m_dragging = true;
}

void HandTool::onMouseMove(double x, double y, int modifiers) {
    if (!m_dragging) return;
    m_delta = {x - m_startPos.x, y - m_startPos.y};
    m_startPos = {x, y};
}

void HandTool::onMouseUp(double x, double y, int modifiers) {
    m_dragging = false;
}

ToolResult HandTool::getResult() const {
    ToolResult result;
    result.consumed = m_dragging;
    if (m_dragging) {
        result.points.push_back(m_delta);
    }
    return result;
}

void HandTool::reset() {
    m_dragging = false;
    m_delta = {0, 0};
}

void ZoomTool::onMouseDown(double x, double y, int modifiers) {
    m_clickPos = {x, y};
    m_zoomIn = !(modifiers & 0x02000000);
}

void ZoomTool::onMouseUp(double x, double y, int modifiers) {
}

ToolResult ZoomTool::getResult() const {
    ToolResult result;
    result.consumed = true;
    result.points.push_back(m_clickPos);
    result.points.push_back({m_zoomIn ? 1.0 : -1.0, 0.0});
    return result;
}

void ZoomTool::reset() {
    m_zoomIn = true;
}

void RotationTool::onMouseDown(double x, double y, int modifiers) {
    m_startPos = {x, y};
    m_rotationDelta = 0.0;
    m_dragging = true;
}

void RotationTool::onMouseMove(double x, double y, int modifiers) {
    if (!m_dragging) return;

    double angle1 = std::atan2(m_startPos.y - m_anchorPoint.y,
                               m_startPos.x - m_anchorPoint.x);
    double angle2 = std::atan2(y - m_anchorPoint.y,
                               x - m_anchorPoint.x);
    m_rotationDelta = (angle2 - angle1) * 180.0 / kPi;
    m_startPos = {x, y};
}

void RotationTool::onMouseUp(double x, double y, int modifiers) {
    m_dragging = false;
}

ToolResult RotationTool::getResult() const {
    ToolResult result;
    result.consumed = m_dragging;
    if (m_dragging) {
        result.points.push_back({m_rotationDelta, 0.0});
    }
    return result;
}

void RotationTool::reset() {
    m_dragging = false;
    m_rotationDelta = 0.0;
}

void PanBehindTool::onMouseDown(double x, double y, int modifiers) {
    m_startPos = {x, y};
    m_delta = {0, 0};
    m_dragging = true;
}

void PanBehindTool::onMouseMove(double x, double y, int modifiers) {
    if (!m_dragging) return;
    m_delta = {x - m_startPos.x, y - m_startPos.y};
    m_startPos = {x, y};
}

void PanBehindTool::onMouseUp(double x, double y, int modifiers) {
    m_dragging = false;
}

ToolResult PanBehindTool::getResult() const {
    ToolResult result;
    result.consumed = m_dragging;
    if (m_dragging) {
        result.points.push_back(m_delta);
    }
    return result;
}

void PanBehindTool::reset() {
    m_dragging = false;
    m_delta = {0, 0};
}

} // namespace FreeEffect
