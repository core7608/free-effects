#pragma once

#include "tool_interface.h"
#include <vector>

namespace FreeEffect {

class PenTool : public Tool {
public:
    PenTool() = default;

    ToolType getType() const override { return ToolType::Pen; }
    std::string getName() const override { return "Pen Tool"; }
    std::string getIcon() const override { return ":/icons/tools/pen.svg"; }

    void onMouseDown(double x, double y, int modifiers) override;
    void onMouseMove(double x, double y, int modifiers) override;
    void onMouseUp(double x, double y, int modifiers) override;
    void onDoubleClick(double x, double y) override;
    void onKeyDown(int key) override;
    ToolResult getResult() const override;

    void reset();

private:
    bool isNearFirstPoint(double x, double y, double threshold = 8.0) const;
    void closePath();
    void convertCornerToSmooth(int index);

    std::vector<BezierPoint> m_points;
    bool m_pathClosed = false;
    bool m_draggingHandle = false;
    int m_draggingPointIndex = -1;
    Point2D m_currentPos;
};

} // namespace FreeEffect
