#pragma once

#include "tool_interface.h"
#include <vector>

namespace FreeEffect {

class FreehandTool : public Tool {
public:
    FreehandTool() = default;

    ToolType getType() const override { return ToolType::Freehand; }
    std::string getName() const override { return "Freehand Tool"; }
    std::string getIcon() const override { return ":/icons/tools/pen.svg"; }

    void onMouseDown(double x, double y, int modifiers) override;
    void onMouseMove(double x, double y, int modifiers) override;
    void onMouseUp(double x, double y, int modifiers) override;
    ToolResult getResult() const override;
    void reset();

    void setSimplifyTolerance(double tolerance) { m_simplifyTolerance = tolerance; }
    double getSimplifyTolerance() const { return m_simplifyTolerance; }

private:
    static double pointToLineDistance(
        const Point2D& p, const Point2D& a, const Point2D& b);

    static void rdpSimplify(
        const std::vector<Point2D>& points, double epsilon,
        std::vector<Point2D>& result);

    std::vector<Point2D> m_rawPoints;
    std::vector<BezierPoint> m_simplifiedPath;
    bool m_drawing = false;
    double m_simplifyTolerance = 2.0;
};

} // namespace FreeEffect
