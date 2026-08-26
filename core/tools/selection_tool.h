#pragma once

#include "tool_interface.h"
#include <vector>
#include <memory>

namespace FreeEffect {

class Layer;

class SelectionTool : public Tool {
public:
    SelectionTool() = default;

    ToolType getType() const override { return ToolType::Select; }
    std::string getName() const override { return "Selection Tool"; }
    std::string getIcon() const override { return ":/icons/tools/selection.svg"; }

    void onMouseDown(double x, double y, int modifiers) override;
    void onMouseMove(double x, double y, int modifiers) override;
    void onMouseUp(double x, double y, int modifiers) override;
    ToolResult getResult() const override;
    void reset();

    void setSelectedLayers(const std::vector<std::shared_ptr<Layer>>& layers) {
        m_selectedLayers = layers;
    }

    enum class DragMode {
        None,
        Move,
        Scale,
        Rotate,
        BoxSelect
    };

    DragMode getCurrentMode() const { return m_dragMode; }
    Point2D getDragDelta() const { return m_dragDelta; }
    double getRotationDelta() const { return m_rotationDelta; }
    double getScaleFactor() const { return m_scaleFactor; }

private:
    std::vector<std::shared_ptr<Layer>> m_selectedLayers;
    DragMode m_dragMode = DragMode::None;
    Point2D m_startPos;
    Point2D m_currentPos;
    Point2D m_dragDelta;
    double m_rotationDelta = 0.0;
    double m_scaleFactor = 1.0;
    bool m_dragging = false;

    struct BoxSelection {
        double x1 = 0, y1 = 0, x2 = 0, y2 = 0;
        bool active = false;
    };
    BoxSelection m_boxSelection;
};

} // namespace FreeEffect
