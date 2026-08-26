#pragma once

#include <string>
#include <vector>

namespace FreeEffect {

struct Point2D { double x = 0, y = 0; };
struct BezierPoint {
    Point2D position;
    Point2D handleIn;
    Point2D handleOut;
    bool smooth = true;
};

enum class ToolType {
    Select, Hand, Zoom, Rotation, PanBehind,
    Rectangle, Ellipse, Polygon, Star, Pen, Freehand,
    Brush, CloneStamp, Eraser,
    PuppetPin, PuppetStretch, PuppetBend, PuppetStarch,
    OrbitCamera, TrackXYCamera, TrackZCamera,
    Type, RotoBrush, RefineEdge
};

struct ToolResult {
    std::vector<BezierPoint> path;
    std::vector<Point2D> points;
    bool consumed = false;
    bool finished = false;
};

class Tool {
public:
    virtual ~Tool() = default;
    virtual ToolType getType() const = 0;
    virtual std::string getName() const = 0;
    virtual std::string getIcon() const { return ""; }

    virtual void onMouseDown(double x, double y, int modifiers) {}
    virtual void onMouseMove(double x, double y, int modifiers) {}
    virtual void onMouseUp(double x, double y, int modifiers) {}
    virtual void onDoubleClick(double x, double y) {}
    virtual void onKeyDown(int key) {}
    virtual ToolResult getResult() const { return {}; }

    void setActive(bool active) { m_active = active; }
    bool isActive() const { return m_active; }

protected:
    bool m_active = false;
};

} // namespace FreeEffect
