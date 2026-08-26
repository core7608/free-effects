#pragma once

#include "types.h"
#include <vector>

namespace FreeEffect {

struct Point2D {
    double x = 0.0;
    double y = 0.0;
    Point2D() = default;
    Point2D(double x, double y) : x(x), y(y) {}
};

struct BezierPoint {
    Point2D position;
    Point2D handleIn;
    Point2D handleOut;
    bool smooth = true;
    BezierPoint() = default;
    BezierPoint(double px, double py) : position(px, py), handleIn(px, py), handleOut(px, py), smooth(true) {}
    BezierPoint(double px, double py, double hix, double hiy, double hox, double hoy)
        : position(px, py), handleIn(hix, hiy), handleOut(hox, hoy), smooth(true) {}
};

class ShapeOperations {
public:
    // Boolean operations (simplified polygon-based)
    static std::vector<BezierPoint> mergePaths(const std::vector<BezierPoint>& pathA,
                                               const std::vector<BezierPoint>& pathB);
    static std::vector<BezierPoint> subtractPaths(const std::vector<BezierPoint>& pathA,
                                                  const std::vector<BezierPoint>& pathB);
    static std::vector<BezierPoint> intersectPaths(const std::vector<BezierPoint>& pathA,
                                                   const std::vector<BezierPoint>& pathB);

    // Path modifications
    static std::vector<BezierPoint> offsetPaths(const std::vector<BezierPoint>& path, double amount);
    static std::vector<BezierPoint> roundCorners(const std::vector<BezierPoint>& path, double radius);
    static std::vector<BezierPoint> puckerAndBloat(const std::vector<BezierPoint>& path, double amount);
    static std::vector<BezierPoint> zigZag(const std::vector<BezierPoint>& path, double size,
                                           int ridges, int side);
    static std::vector<BezierPoint> twist(const std::vector<BezierPoint>& path, double angle);
    static std::vector<BezierPoint> warpStarch(const std::vector<BezierPoint>& path, double amount);

    // Path animation
    static std::vector<BezierPoint> trimPaths(const std::vector<BezierPoint>& path,
                                              double start, double end, double offset);
    static std::vector<BezierPoint> trimPathAnimated(const std::vector<BezierPoint>& path,
                                                     double start, double end, double offset, double time);

    // Repeat
    static std::vector<std::vector<BezierPoint>> repeatShape(const std::vector<BezierPoint>& path,
        int copies, double offset, double startOpacity, double endOpacity, double rotation);

    // Path simplification (Ramer-Douglas-Peucker)
    static std::vector<BezierPoint> simplifyPath(const std::vector<BezierPoint>& path, double tolerance);

    // Path to points conversion
    static std::vector<Point2D> pathToPoints(const std::vector<BezierPoint>& path, int subdivisions = 8);

    // Point inside polygon test
    static bool isPointInsidePolygon(double x, double y, const std::vector<Point2D>& polygon);

    // Path length
    static double pathLength(const std::vector<BezierPoint>& path);

    // Point at distance along path
    static Point2D pointAtDistance(const std::vector<BezierPoint>& path, double distance);
};

} // namespace FreeEffect
