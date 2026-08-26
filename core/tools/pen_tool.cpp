#include "pen_tool.h"
#include <cmath>
#include <algorithm>

namespace FreeEffect {

void PenTool::onMouseDown(double x, double y, int modifiers) {
    Point2D pos{x, y};

    if (!m_points.empty() && isNearFirstPoint(x, y)) {
        closePath();
        m_pathClosed = true;
        return;
    }

    BezierPoint bp;
    bp.position = pos;
    bp.handleIn = pos;
    bp.handleOut = pos;
    bp.smooth = true;

    m_points.push_back(bp);
    m_draggingHandle = true;
    m_draggingPointIndex = static_cast<int>(m_points.size()) - 1;
}

void PenTool::onMouseMove(double x, double y, int modifiers) {
    m_currentPos = {x, y};

    if (m_draggingHandle && m_draggingPointIndex >= 0 &&
        m_draggingPointIndex < static_cast<int>(m_points.size())) {

        auto& bp = m_points[m_draggingPointIndex];
        bp.handleOut = {x, y};

        if (!(modifiers & 0x04000000)) {
            double dx = bp.handleOut.x - bp.position.x;
            double dy = bp.handleOut.y - bp.position.y;
            bp.handleIn = {bp.position.x - dx, bp.position.y - dy};
        } else {
            bp.smooth = false;
        }
    }
}

void PenTool::onMouseUp(double x, double y, int modifiers) {
    m_draggingHandle = false;
    m_draggingPointIndex = -1;
}

void PenTool::onDoubleClick(double x, double y) {
    if (!m_points.empty()) {
        m_points.pop_back();
    }
    closePath();
}

void PenTool::onKeyDown(int key) {
    if (key == 0x01000005) {
        closePath();
    } else if (key == 0x01000012) {
        reset();
    }
}

ToolResult PenTool::getResult() const {
    ToolResult result;
    result.path = m_points;
    result.finished = m_pathClosed;
    result.consumed = !m_points.empty();
    return result;
}

void PenTool::reset() {
    m_points.clear();
    m_pathClosed = false;
    m_draggingHandle = false;
    m_draggingPointIndex = -1;
}

bool PenTool::isNearFirstPoint(double x, double y, double threshold) const {
    if (m_points.empty()) return false;
    const auto& first = m_points.front().position;
    double dx = x - first.x;
    double dy = y - first.y;
    return std::sqrt(dx * dx + dy * dy) <= threshold;
}

void PenTool::closePath() {
    m_pathClosed = true;
}

void PenTool::convertCornerToSmooth(int index) {
    if (index < 0 || index >= static_cast<int>(m_points.size())) return;

    auto& bp = m_points[index];
    double dxOut = bp.handleOut.x - bp.position.x;
    double dyOut = bp.handleOut.y - bp.position.y;
    double lenOut = std::sqrt(dxOut * dxOut + dyOut * dyOut);
    double dxIn = bp.handleIn.x - bp.position.x;
    double dyIn = bp.handleIn.y - bp.position.y;
    double lenIn = std::sqrt(dxIn * dxIn + dyIn * dyIn);

    if (lenOut > 1e-6 && lenIn > 1e-6) {
        double avgLen = (lenOut + lenIn) * 0.5;
        double angle = std::atan2(dyOut, dxOut);
        bp.handleOut = {bp.position.x + std::cos(angle) * avgLen,
                        bp.position.y + std::sin(angle) * avgLen};
        bp.handleIn = {bp.position.x - std::cos(angle) * avgLen,
                       bp.position.y - std::sin(angle) * avgLen};
        bp.smooth = true;
    }
}

} // namespace FreeEffect
