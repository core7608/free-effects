#pragma once

#include "tool_interface.h"
#include <vector>

namespace FreeEffect {

class ShapeToolBase : public Tool {
public:
    ShapeToolBase() = default;
    virtual ~ShapeToolBase() = default;

    void onMouseDown(double x, double y, int modifiers) override;
    void onMouseMove(double x, double y, int modifiers) override;
    void onMouseUp(double x, double y, int modifiers) override;
    ToolResult getResult() const override;
    void reset();

protected:
    virtual std::vector<BezierPoint> buildShapePoints(
        double x1, double y1, double x2, double y2, int sides) const = 0;

    Point2D m_start;
    Point2D m_end;
    bool m_drawing = false;
    bool m_constrain = false;
    bool m_centerOut = false;
    std::vector<BezierPoint> m_currentPath;
    int m_polygonSides = 5;
};

class RectangleTool : public ShapeToolBase {
public:
    ToolType getType() const override { return ToolType::Rectangle; }
    std::string getName() const override { return "Rectangle Tool"; }
    std::string getIcon() const override { return ":/icons/tools/shape.svg"; }

protected:
    std::vector<BezierPoint> buildShapePoints(
        double x1, double y1, double x2, double y2, int sides) const override;
};

class EllipseTool : public ShapeToolBase {
public:
    ToolType getType() const override { return ToolType::Ellipse; }
    std::string getName() const override { return "Ellipse Tool"; }
    std::string getIcon() const override { return ":/icons/tools/shape.svg"; }

protected:
    std::vector<BezierPoint> buildShapePoints(
        double x1, double y1, double x2, double y2, int sides) const override;
};

class PolygonTool : public ShapeToolBase {
public:
    ToolType getType() const override { return ToolType::Polygon; }
    std::string getName() const override { return "Polygon Tool"; }
    std::string getIcon() const override { return ":/icons/tools/shape.svg"; }

    void onKeyDown(int key) override;

protected:
    std::vector<BezierPoint> buildShapePoints(
        double x1, double y1, double x2, double y2, int sides) const override;
};

class StarTool : public ShapeToolBase {
public:
    ToolType getType() const override { return ToolType::Star; }
    std::string getName() const override { return "Star Tool"; }
    std::string getIcon() const override { return ":/icons/tools/shape.svg"; }

    void onKeyDown(int key) override;

protected:
    std::vector<BezierPoint> buildShapePoints(
        double x1, double y1, double x2, double y2, int sides) const override;
};

} // namespace FreeEffect
