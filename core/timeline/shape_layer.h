#pragma once

#include "../rendering/renderer.h"
#include "types.h"
#include <string>
#include <vector>

namespace FreeEffect {

struct BezierCurvePath {
    Vec2 startPoint;
    Vec2 endPoint;
    Vec2 controlPoint1;
    Vec2 controlPoint2;

    Vec2 evaluate(double t) const {
        double u = 1.0 - t;
        double tt = t * t;
        double uu = u * u;
        double uuu = uu * u;
        double ttt = tt * t;

        Vec2 result;
        result.x = uuu * startPoint.x + 3.0 * uu * t * controlPoint1.x +
                   3.0 * u * tt * controlPoint2.x + ttt * endPoint.x;
        result.y = uuu * startPoint.y + 3.0 * uu * t * controlPoint1.y +
                   3.0 * u * tt * controlPoint2.y + ttt * endPoint.y;
        return result;
    }
};

struct ShapePath {
    std::vector<Vec2> points;
    std::vector<BezierCurvePath> curves;
    bool closed = true;
};

enum class ShapeType {
    Rectangle,
    Ellipse,
    Polystar,
    Path,
    Star
};

struct ShapeGroup {
    std::string name;
    std::vector<ShapePath> paths;
    Color fillColor{1.0, 1.0, 1.0, 1.0};
    Color strokeColor{0.0, 0.0, 0.0, 0.0};
    double strokeWidth = 0.0;
    bool fillEnabled = true;
    bool strokeEnabled = false;
    int blendMode = 0;
    std::vector<ShapeGroup> groups;

    static ShapeGroup createRectangle(double x, double y, double w, double h, double roundness = 0.0);
    static ShapeGroup createEllipse(double cx, double cy, double rx, double ry);
    static ShapeGroup createStar(double cx, double cy, double outerRadius, double innerRadius, int points, double rotation = 0.0);
};

class ShapeLayerData {
public:
    ShapeLayerData() = default;

    std::vector<ShapeGroup>& getContents() { return m_contents; }
    const std::vector<ShapeGroup>& getContents() const { return m_contents; }

    void addGroup(const ShapeGroup& group);
    void removeGroup(int index);

    PixelBuffer rasterize(int width, int height) const;
    std::vector<ShapePath> getVectorPaths() const;

private:
    void rasterizeGroup(const ShapeGroup& group, PixelBuffer& buffer) const;
    void rasterizePath(const ShapePath& path, const Color& fillColor,
                       const Color& strokeColor, double strokeWidth,
                       PixelBuffer& buffer) const;
    bool isPointInsidePath(double px, double py, const ShapePath& path) const;
    void getGroupPaths(const ShapeGroup& group, std::vector<ShapePath>& out) const;

    std::vector<ShapeGroup> m_contents;
};

} // namespace FreeEffect
