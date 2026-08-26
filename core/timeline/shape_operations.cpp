#include "shape_operations.h"
#include <cmath>
#include <algorithm>
#include <numeric>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace FreeEffect {

// ======================== Helpers ========================

static double dist2D(const Point2D& a, const Point2D& b) {
    double dx = b.x - a.x;
    double dy = b.y - a.y;
    return std::sqrt(dx * dx + dy * dy);
}

static Point2D lerp2D(const Point2D& a, const Point2D& b, double t) {
    return {a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t};
}

static Point2D bezierEval(const Point2D& p0, const Point2D& p1,
                          const Point2D& p2, const Point2D& p3, double t) {
    double u = 1.0 - t;
    double uu = u * u;
    double uuu = uu * u;
    double tt = t * t;
    double ttt = tt * t;
    Point2D r;
    r.x = uuu * p0.x + 3.0 * uu * t * p1.x + 3.0 * u * tt * p2.x + ttt * p3.x;
    r.y = uuu * p0.y + 3.0 * uu * t * p1.y + 3.0 * u * tt * p2.y + ttt * p3.y;
    return r;
}

static double cross2D(const Point2D& o, const Point2D& a, const Point2D& b) {
    return (a.x - o.x) * (b.y - o.y) - (a.y - o.y) * (b.x - o.x);
}

// Get segment between consecutive bezier points
struct Segment {
    Point2D start, cp1, cp2, end;
};

static std::vector<Segment> getSegments(const std::vector<BezierPoint>& path) {
    std::vector<Segment> segs;
    if (path.size() < 2) return segs;
    for (size_t i = 0; i + 1 < path.size(); i++) {
        Segment s;
        s.start = path[i].position;
        s.cp1 = path[i].handleOut;
        s.cp2 = path[i + 1].handleIn;
        s.end = path[i + 1].position;
        segs.push_back(s);
    }
    return segs;
}

static std::vector<BezierPoint> segmentsToPath(const std::vector<Segment>& segs) {
    std::vector<BezierPoint> path;
    if (segs.empty()) return path;
    for (size_t i = 0; i < segs.size(); i++) {
        BezierPoint bp;
        bp.position = segs[i].start;
        bp.handleIn = (i > 0) ? segs[i - 1].cp2 : segs[i].cp1;
        bp.handleOut = segs[i].cp1;
        path.push_back(bp);
    }
    // Last point
    BezierPoint last;
    last.position = segs.back().end;
    last.handleIn = segs.back().cp2;
    last.handleOut = segs.back().end;
    path.push_back(last);
    return path;
}

// ======================== Boolean Operations ========================

std::vector<BezierPoint> ShapeOperations::mergePaths(const std::vector<BezierPoint>& pathA,
                                                      const std::vector<BezierPoint>& pathB) {
    // Simplified merge: combine both paths into one
    std::vector<BezierPoint> result = pathA;
    if (!pathA.empty() && !pathB.empty()) {
        // Connect end of A to start of B with a line
        BezierPoint bridge;
        bridge.position = pathB.front().position;
        bridge.handleIn = pathA.back().position;
        bridge.handleOut = pathB.front().handleOut;
        result.push_back(bridge);
    }
    result.insert(result.end(), pathB.begin(), pathB.end());
    return result;
}

std::vector<BezierPoint> ShapeOperations::subtractPaths(const std::vector<BezierPoint>& pathA,
                                                         const std::vector<BezierPoint>& pathB) {
    // Simplified: keep points of A that are outside B
    std::vector<Point2D> polyB = pathToPoints(pathB);
    std::vector<BezierPoint> result;
    for (const auto& bp : pathA) {
        if (!isPointInsidePolygon(bp.position.x, bp.position.y, polyB)) {
            result.push_back(bp);
        }
    }
    if (result.empty()) result = pathA;
    return result;
}

std::vector<BezierPoint> ShapeOperations::intersectPaths(const std::vector<BezierPoint>& pathA,
                                                          const std::vector<BezierPoint>& pathB) {
    // Simplified: keep points of A that are inside B
    std::vector<Point2D> polyB = pathToPoints(pathB);
    std::vector<BezierPoint> result;
    for (const auto& bp : pathA) {
        if (isPointInsidePolygon(bp.position.x, bp.position.y, polyB)) {
            result.push_back(bp);
        }
    }
    if (result.empty()) result = pathA;
    return result;
}

// ======================== Path Modifications ========================

std::vector<BezierPoint> ShapeOperations::offsetPaths(const std::vector<BezierPoint>& path, double amount) {
    if (path.size() < 2) return path;
    std::vector<BezierPoint> result = path;
    size_t n = path.size();

    for (size_t i = 0; i < n; i++) {
        // Compute normals from adjacent segments
        Point2D prev = path[(i + n - 1) % n].position;
        Point2D curr = path[i].position;
        Point2D next = path[(i + 1) % n].position;

        double dx1 = curr.x - prev.x;
        double dy1 = curr.y - prev.y;
        double dx2 = next.x - curr.x;
        double dy2 = next.y - curr.y;

        // Average normal (perpendicular to average tangent)
        double tx = dx1 + dx2;
        double ty = dy1 + dy2;
        double len = std::sqrt(tx * tx + ty * ty);
        if (len < 1e-12) { len = 1.0; tx = 1.0; ty = 0.0; }

        // Normal: perpendicular (rotate 90 degrees)
        double nx = -ty / len;
        double ny = tx / len;

        result[i].position.x = curr.x + nx * amount;
        result[i].position.y = curr.y + ny * amount;
        result[i].handleIn.x = path[i].handleIn.x + nx * amount;
        result[i].handleIn.y = path[i].handleIn.y + ny * amount;
        result[i].handleOut.x = path[i].handleOut.x + nx * amount;
        result[i].handleOut.y = path[i].handleOut.y + ny * amount;
    }
    return result;
}

std::vector<BezierPoint> ShapeOperations::roundCorners(const std::vector<BezierPoint>& path, double radius) {
    if (path.size() < 3 || radius <= 0.0) return path;
    std::vector<BezierPoint> result;
    size_t n = path.size();

    for (size_t i = 0; i < n; i++) {
        Point2D prev = path[(i + n - 1) % n].position;
        Point2D curr = path[i].position;
        Point2D next = path[(i + 1) % n].position;

        double d1 = dist2D(curr, prev);
        double d2 = dist2D(curr, next);
        double r1 = std::min(radius, d1 / 2.0);
        double r2 = std::min(radius, d2 / 2.0);

        // Points along edges at radius distance
        Point2D p1 = lerp2D(curr, prev, r1 / d1);
        Point2D p2 = lerp2D(curr, next, r2 / d2);

        // Control points for cubic bezier approximating arc
        double kappa = 0.5522847498;
        Point2D cp1 = lerp2D(p1, curr, 1.0 - kappa);
        Point2D cp2 = lerp2D(p2, curr, 1.0 - kappa);

        BezierPoint bp;
        bp.position = p1;
        bp.handleIn = path[i].handleIn;
        bp.handleOut = cp1;
        result.push_back(bp);

        BezierPoint arc;
        arc.position = p2;
        arc.handleIn = cp2;
        arc.handleOut = path[i].handleOut;
        result.push_back(arc);
    }
    return result;
}

std::vector<BezierPoint> ShapeOperations::puckerAndBloat(const std::vector<BezierPoint>& path, double amount) {
    if (path.size() < 2) return path;
    std::vector<BezierPoint> result = path;
    size_t n = path.size();

    for (size_t i = 0; i < n; i++) {
        Point2D prev = path[(i + n - 1) % n].position;
        Point2D curr = path[i].position;
        Point2D next = path[(i + 1) % n].position;

        double dx = (next.x - prev.x) * 0.5;
        double dy = (next.y - prev.y) * 0.5;
        double len = std::sqrt(dx * dx + dy * dy);
        if (len < 1e-12) continue;

        // Move handleIn and handleOut toward/away from midpoint
        double factor = amount * 0.01;
        result[i].handleIn.x = path[i].handleIn.x + dx * factor;
        result[i].handleIn.y = path[i].handleIn.y + dy * factor;
        result[i].handleOut.x = path[i].handleOut.x - dx * factor;
        result[i].handleOut.y = path[i].handleOut.y - dy * factor;
    }
    return result;
}

std::vector<BezierPoint> ShapeOperations::zigZag(const std::vector<BezierPoint>& path,
                                                  double size, int ridges, int side) {
    if (path.size() < 2 || ridges <= 0) return path;
    std::vector<BezierPoint> result;
    size_t n = path.size();

    for (size_t i = 0; i < n; i++) {
        Point2D prev = path[(i + n - 1) % n].position;
        Point2D curr = path[i].position;
        Point2D next = path[(i + 1) % n].position;

        double dx = next.x - curr.x;
        double dy = next.y - curr.y;
        double len = std::sqrt(dx * dx + dy * dy);
        if (len < 1e-12) { result.push_back(path[i]); continue; }

        double nx = -dy / len;
        double ny = dx / len;

        // Zigzag offset
        double t = static_cast<double>(i) / static_cast<double>(n);
        double zigzag = std::sin(t * ridges * 2.0 * M_PI) * size;
        if (side == 1) zigzag = std::abs(zigzag);
        else if (side == 2) zigzag = -std::abs(zigzag);

        BezierPoint bp = path[i];
        bp.position.x += nx * zigzag;
        bp.position.y += ny * zigzag;
        result.push_back(bp);
    }
    return result;
}

std::vector<BezierPoint> ShapeOperations::twist(const std::vector<BezierPoint>& path, double angle) {
    if (path.empty()) return path;
    // Compute centroid
    double cx = 0, cy = 0;
    for (const auto& bp : path) { cx += bp.position.x; cy += bp.position.y; }
    cx /= path.size();
    cy /= path.size();

    double rad = angle * M_PI / 180.0;
    std::vector<BezierPoint> result = path;
    for (size_t i = 0; i < path.size(); i++) {
        double t = static_cast<double>(i) / static_cast<double>(path.size());
        double localAngle = rad * t;
        double cosA = std::cos(localAngle);
        double sinA = std::sin(localAngle);

        auto rotatePoint = [&](Point2D p) -> Point2D {
            double dx = p.x - cx;
            double dy = p.y - cy;
            return {cx + dx * cosA - dy * sinA, cy + dx * sinA + dy * cosA};
        };
        result[i].position = rotatePoint(path[i].position);
        result[i].handleIn = rotatePoint(path[i].handleIn);
        result[i].handleOut = rotatePoint(path[i].handleOut);
    }
    return result;
}

std::vector<BezierPoint> ShapeOperations::warpStarch(const std::vector<BezierPoint>& path, double amount) {
    if (path.size() < 2) return path;
    std::vector<BezierPoint> result = path;
    size_t n = path.size();

    for (size_t i = 0; i < n; i++) {
        double t = static_cast<double>(i) / static_cast<double>(n - 1);
        double factor = std::sin(t * M_PI) * amount * 0.01;

        // Move handle points toward/away from position
        result[i].handleIn.x = path[i].position.x + (path[i].handleIn.x - path[i].position.x) * (1.0 + factor);
        result[i].handleIn.y = path[i].position.y + (path[i].handleIn.y - path[i].position.y) * (1.0 + factor);
        result[i].handleOut.x = path[i].position.x + (path[i].handleOut.x - path[i].position.x) * (1.0 + factor);
        result[i].handleOut.y = path[i].position.y + (path[i].handleOut.y - path[i].position.y) * (1.0 + factor);
    }
    return result;
}

// ======================== Path Animation ========================

std::vector<BezierPoint> ShapeOperations::trimPaths(const std::vector<BezierPoint>& path,
                                                     double start, double end, double offset) {
    if (path.size() < 2) return path;
    double totalLen = pathLength(path);
    if (totalLen < 1e-12) return path;

    double s = std::clamp(start + offset, 0.0, 1.0);
    double e = std::clamp(end + offset, 0.0, 1.0);
    if (s > e) std::swap(s, e);

    double startDist = s * totalLen;
    double endDist = e * totalLen;

    std::vector<Point2D> points = pathToPoints(path, 16);
    std::vector<BezierPoint> result;
    bool inRange = false;

    double cumDist = 0.0;
    for (size_t i = 0; i < points.size(); i++) {
        if (i > 0) cumDist += dist2D(points[i - 1], points[i]);
        if (cumDist >= startDist && cumDist <= endDist) {
            BezierPoint bp(points[i].x, points[i].y);
            result.push_back(bp);
        }
    }

    // Ensure at least start and end points
    if (result.size() < 2 && points.size() >= 2) {
        result.clear();
        result.emplace_back(points.front().x, points.front().y);
        result.emplace_back(points.back().x, points.back().y);
    }

    return result;
}

std::vector<BezierPoint> ShapeOperations::trimPathAnimated(const std::vector<BezierPoint>& path,
                                                            double start, double end, double offset, double time) {
    double animatedOffset = offset + time * 0.1;
    return trimPaths(path, start, end, animatedOffset);
}

// ======================== Repeat ========================

std::vector<std::vector<BezierPoint>> ShapeOperations::repeatShape(
    const std::vector<BezierPoint>& path, int copies, double offset,
    double startOpacity, double endOpacity, double rotation) {
    std::vector<std::vector<BezierPoint>> result;
    if (path.empty() || copies < 1) return result;

    // Centroid for rotation
    double cx = 0, cy = 0;
    for (const auto& bp : path) { cx += bp.position.x; cy += bp.position.y; }
    cx /= path.size();
    cy /= path.size();

    double totalRot = rotation * M_PI / 180.0;

    for (int i = 0; i < copies; i++) {
        double t = (copies > 1) ? static_cast<double>(i) / static_cast<double>(copies - 1) : 0.0;
        double rot = totalRot * t;
        double off = offset * i;
        double cosR = std::cos(rot);
        double sinR = std::sin(rot);

        std::vector<BezierPoint> copy;
        for (const auto& bp : path) {
            BezierPoint nbp = bp;
            // Rotate around centroid
            double dx = bp.position.x - cx;
            double dy = bp.position.y - cy;
            nbp.position.x = cx + dx * cosR - dy * sinR + off;
            nbp.position.y = cy + dx * sinR + dy * cosR;
            nbp.handleIn.x = bp.handleIn.x + off;
            nbp.handleIn.y = bp.handleIn.y;
            nbp.handleOut.x = bp.handleOut.x + off;
            nbp.handleOut.y = bp.handleOut.y;
            copy.push_back(nbp);
        }
        result.push_back(copy);
    }
    return result;
}

// ======================== Path Simplification (RDP) ========================

static double perpendicularDistance(const Point2D& p, const Point2D& lineStart, const Point2D& lineEnd) {
    double dx = lineEnd.x - lineStart.x;
    double dy = lineEnd.y - lineStart.y;
    double lenSq = dx * dx + dy * dy;
    if (lenSq < 1e-12) return dist2D(p, lineStart);
    double t = std::clamp(((p.x - lineStart.x) * dx + (p.y - lineStart.y) * dy) / lenSq, 0.0, 1.0);
    Point2D proj = {lineStart.x + t * dx, lineStart.y + t * dy};
    return dist2D(p, proj);
}

static void rdpSimplify(const std::vector<Point2D>& points, double tolerance,
                         std::vector<Point2D>& result, size_t start, size_t end) {
    if (end <= start + 1) {
        if (start < points.size()) result.push_back(points[start]);
        return;
    }
    double maxDist = 0.0;
    size_t maxIdx = start;
    for (size_t i = start + 1; i < end; i++) {
        double d = perpendicularDistance(points[i], points[start], points[end]);
        if (d > maxDist) { maxDist = d; maxIdx = i; }
    }
    if (maxDist > tolerance) {
        rdpSimplify(points, tolerance, result, start, maxIdx);
        rdpSimplify(points, tolerance, result, maxIdx, end);
    } else {
        result.push_back(points[start]);
    }
}

std::vector<BezierPoint> ShapeOperations::simplifyPath(const std::vector<BezierPoint>& path, double tolerance) {
    if (path.size() <= 2) return path;
    std::vector<Point2D> pts;
    for (const auto& bp : path) pts.push_back(bp.position);
    std::vector<Point2D> simplified;
    rdpSimplify(pts, tolerance, simplified, 0, pts.size() - 1);
    if (!simplified.empty() && pts.size() > 1) {
        simplified.push_back(pts.back());
    }
    std::vector<BezierPoint> result;
    for (const auto& p : simplified) {
        result.emplace_back(p.x, p.y);
    }
    return result;
}

// ======================== Path to Points ========================

std::vector<Point2D> ShapeOperations::pathToPoints(const std::vector<BezierPoint>& path, int subdivisions) {
    std::vector<Point2D> points;
    if (path.empty()) return points;

    auto segs = getSegments(path);
    for (const auto& seg : segs) {
        for (int i = 0; i <= subdivisions; i++) {
            double t = static_cast<double>(i) / static_cast<double>(subdivisions);
            points.push_back(bezierEval(seg.start, seg.cp1, seg.cp2, seg.end, t));
        }
    }
    // Remove duplicate consecutive points
    std::vector<Point2D> unique;
    for (const auto& p : points) {
        if (unique.empty() || dist2D(unique.back(), p) > 1e-6) {
            unique.push_back(p);
        }
    }
    return unique;
}

// ======================== Point Inside Polygon ========================

bool ShapeOperations::isPointInsidePolygon(double x, double y, const std::vector<Point2D>& polygon) {
    if (polygon.size() < 3) return false;
    bool inside = false;
    size_t n = polygon.size();
    for (size_t i = 0, j = n - 1; i < n; j = i++) {
        const Point2D& pi = polygon[i];
        const Point2D& pj = polygon[j];
        if (((pi.y > y) != (pj.y > y)) &&
            (x < (pj.x - pi.x) * (y - pi.y) / (pj.y - pi.y) + pi.x)) {
            inside = !inside;
        }
    }
    return inside;
}

// ======================== Path Length ========================

double ShapeOperations::pathLength(const std::vector<BezierPoint>& path) {
    if (path.size() < 2) return 0.0;
    double total = 0.0;
    auto segs = getSegments(path);
    const int steps = 16;
    for (const auto& seg : segs) {
        Point2D prev = seg.start;
        for (int i = 1; i <= steps; i++) {
            double t = static_cast<double>(i) / static_cast<double>(steps);
            Point2D p = bezierEval(seg.start, seg.cp1, seg.cp2, seg.end, t);
            total += dist2D(prev, p);
            prev = p;
        }
    }
    return total;
}

// ======================== Point at Distance ========================

Point2D ShapeOperations::pointAtDistance(const std::vector<BezierPoint>& path, double distance) {
    if (path.empty()) return {0, 0};
    if (path.size() == 1) return path[0].position;
    if (distance <= 0) return path.front().position;

    auto segs = getSegments(path);
    const int steps = 16;
    double cumDist = 0.0;

    for (const auto& seg : segs) {
        Point2D prev = seg.start;
        for (int i = 1; i <= steps; i++) {
            double t = static_cast<double>(i) / static_cast<double>(steps);
            Point2D p = bezierEval(seg.start, seg.cp1, seg.cp2, seg.end, t);
            double d = dist2D(prev, p);
            if (cumDist + d >= distance) {
                double remain = distance - cumDist;
                double localT = (d > 1e-12) ? remain / d : 0.0;
                return lerp2D(prev, p, localT);
            }
            cumDist += d;
            prev = p;
        }
    }
    return path.back().position;
}

} // namespace FreeEffect
