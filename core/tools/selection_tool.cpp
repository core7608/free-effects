#include "selection_tool.h"
#include "../timeline/layer.h"
#include <cmath>
#include <algorithm>

namespace FreeEffect {

void SelectionTool::onMouseDown(double x, double y, int modifiers) {
    m_startPos = {x, y};
    m_currentPos = {x, y};
    m_dragging = true;
    m_dragDelta = {0, 0};
    m_rotationDelta = 0.0;
    m_scaleFactor = 1.0;

    if (m_selectedLayers.empty()) {
        m_dragMode = DragMode::BoxSelect;
        m_boxSelection = {x, y, x, y, true};
    } else {
        m_dragMode = DragMode::Move;
    }

    if (modifiers & 0x02000000) {
        m_dragMode = DragMode::Scale;
    } else if (modifiers & 0x04000000) {
        m_dragMode = DragMode::Rotate;
    }
}

void SelectionTool::onMouseMove(double x, double y, int modifiers) {
    if (!m_dragging) return;
    m_currentPos = {x, y};

    switch (m_dragMode) {
        case DragMode::Move:
            m_dragDelta = {m_currentPos.x - m_startPos.x,
                           m_currentPos.y - m_startPos.y};
            break;

        case DragMode::Rotate: {
            double cx = m_startPos.x;
            double cy = m_startPos.y;
            double angle1 = std::atan2(m_startPos.y - cy, m_startPos.x - cx);
            double angle2 = std::atan2(m_currentPos.y - cy, m_currentPos.x - cx);
            m_rotationDelta = (angle2 - angle1) * 180.0 / 3.14159265358979323846;
            break;
        }

        case DragMode::Scale: {
            double dxStart = m_startPos.x;
            double dyStart = m_startPos.y;
            double startDist = std::sqrt(dxStart * dxStart + dyStart * dyStart);
            double dxCurr = m_currentPos.x;
            double dyCurr = m_currentPos.y;
            double currDist = std::sqrt(dxCurr * dxCurr + dyCurr * dyCurr);
            if (startDist > 1e-6) {
                m_scaleFactor = currDist / startDist;
            }
            break;
        }

        case DragMode::BoxSelect:
            m_boxSelection.x2 = m_currentPos.x;
            m_boxSelection.y2 = m_currentPos.y;
            break;

        default:
            break;
    }
}

void SelectionTool::onMouseUp(double x, double y, int modifiers) {
    m_dragging = false;
    if (m_dragMode == DragMode::BoxSelect) {
        m_boxSelection.active = false;
    }
}

ToolResult SelectionTool::getResult() const {
    ToolResult result;
    result.consumed = m_dragging;

    if (m_dragMode == DragMode::Move && m_dragging) {
        result.points.push_back(m_dragDelta);
    }
    return result;
}

void SelectionTool::reset() {
    m_dragging = false;
    m_dragMode = DragMode::None;
    m_dragDelta = {0, 0};
    m_rotationDelta = 0.0;
    m_scaleFactor = 1.0;
    m_boxSelection = {};
}

} // namespace FreeEffect
