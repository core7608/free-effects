#include "shape_layer.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

ShapeGroup ShapeGroup::createRectangle(double x, double y, double w, double h, double roundness) {
    ShapeGroup group;
    group.name = "Rectangle";
    group.fillEnabled = true;
    group.strokeEnabled = false;

    ShapePath path;
    if (roundness < 1e-6) {
        path.points = {
            {x, y}, {x + w, y}, {x + w, y + h}, {x, y + h}
        };
        path.closed = true;
    } else {
        double r = std::min(roundness, std::min(w, h) * 0.5);
        int segs = 8;
        auto addCorner = [&](double cx, double cy, double startAngle) {
            for (int i = 0; i <= segs; ++i) {
                double angle = startAngle + (3.14159265358979323846 * 0.5 * i) / segs;
                path.points.push_back({cx + r * std::cos(angle), cy + r * std::sin(angle)});
            }
        };
        addCorner(x + r, y + r, 3.14159265358979323846);
        addCorner(x + w - r, y + r, 3.14159265358979323846 * 0.5);
        addCorner(x + w - r, y + h - r, 0.0);
        addCorner(x + r, y + h - r, -3.14159265358979323846 * 0.5);
        path.closed = true;
    }

    group.paths.push_back(path);
    return group;
}

ShapeGroup ShapeGroup::createEllipse(double cx, double cy, double rx, double ry) {
    ShapeGroup group;
    group.name = "Ellipse";
    group.fillEnabled = true;
    group.strokeEnabled = false;

    ShapePath path;
    int numSegments = 36;
    path.points.reserve(numSegments);
    for (int i = 0; i < numSegments; ++i) {
        double angle = (2.0 * 3.14159265358979323846 * i) / numSegments;
        path.points.push_back({cx + rx * std::cos(angle), cy + ry * std::sin(angle)});
    }
    path.closed = true;

    group.paths.push_back(path);
    return group;
}

ShapeGroup ShapeGroup::createStar(double cx, double cy, double outerRadius, double innerRadius, int points, double rotation) {
    ShapeGroup group;
    group.name = "Star";
    group.fillEnabled = true;
    group.strokeEnabled = false;

    ShapePath path;
    int totalPoints = points * 2;
    double rotRad = rotation * 3.14159265358979323846 / 180.0;
    path.points.reserve(totalPoints);
    for (int i = 0; i < totalPoints; ++i) {
        double angle = rotRad + (2.0 * 3.14159265358979323846 * i) / totalPoints - 3.14159265358979323846 * 0.5;
        double r = (i % 2 == 0) ? outerRadius : innerRadius;
        path.points.push_back({cx + r * std::cos(angle), cy + r * std::sin(angle)});
    }
    path.closed = true;

    group.paths.push_back(path);
    return group;
}

void ShapeLayerData::addGroup(const ShapeGroup& group) {
    m_contents.push_back(group);
}

void ShapeLayerData::removeGroup(int index) {
    if (index >= 0 && index < static_cast<int>(m_contents.size())) {
        m_contents.erase(m_contents.begin() + index);
    }
}

bool ShapeLayerData::isPointInsidePath(double px, double py, const ShapePath& path) const {
    int n = static_cast<int>(path.points.size());
    if (n < 3) return false;

    int crossings = 0;
    for (int i = 0; i < n; ++i) {
        int next = (i + 1) % n;
        const Vec2& p0 = path.points[i];
        const Vec2& p1 = path.points[next];

        bool hasBezier = (i < static_cast<int>(path.curves.size()));
        if (hasBezier) {
            const BezierCurvePath& curve = path.curves[i];
            Vec2 prevPt = p0;
            for (int s = 1; s <= 10; ++s) {
                double t = static_cast<double>(s) / 10.0;
                Vec2 currPt = curve.evaluate(t);
                if ((prevPt.y > py) != (currPt.y > py)) {
                    double xIntersect = prevPt.x + (currPt.x - prevPt.x) *
                                        (py - prevPt.y) / (currPt.y - prevPt.y);
                    if (px < xIntersect) crossings++;
                }
                prevPt = currPt;
            }
        } else {
            if ((p0.y > py) != (p1.y > py)) {
                double xIntersect = p0.x + (p1.x - p0.x) * (py - p0.y) / (p1.y - p0.y);
                if (px < xIntersect) crossings++;
            }
        }
    }

    return (crossings % 2) == 1;
}

void ShapeLayerData::rasterizePath(const ShapePath& path, const Color& fillColor,
                                    const Color& strokeColor, double strokeWidth,
                                    PixelBuffer& buffer) const {
    int w = buffer.width;
    int h = buffer.height;

    if (w <= 0 || h <= 0 || path.points.empty()) return;

    double minX = 1e10, minY = 1e10, maxX = -1e10, maxY = -1e10;
    for (const auto& pt : path.points) {
        minX = std::min(minX, pt.x);
        minY = std::min(minY, pt.y);
        maxX = std::max(maxX, pt.x);
        maxY = std::max(maxY, pt.y);
    }

    int startX = std::max(0, static_cast<int>(minX) - static_cast<int>(strokeWidth) - 1);
    int startY = std::max(0, static_cast<int>(minY) - static_cast<int>(strokeWidth) - 1);
    int endX = std::min(w - 1, static_cast<int>(maxX) + static_cast<int>(strokeWidth) + 1);
    int endY = std::min(h - 1, static_cast<int>(maxY) + static_cast<int>(strokeWidth) + 1);

    for (int y = startY; y <= endY; ++y) {
        for (int x = startX; x <= endX; ++x) {
            bool inside = isPointInsidePath(static_cast<double>(x) + 0.5,
                                            static_cast<double>(y) + 0.5, path);
            uint8_t* dst = buffer.pixelAt(x, y);

            if (inside && fillColor.a > 1e-6) {
                double alpha = fillColor.a;
                dst[0] = static_cast<uint8_t>(fillColor.r * 255.0 * alpha + dst[0] * (1.0 - alpha));
                dst[1] = static_cast<uint8_t>(fillColor.g * 255.0 * alpha + dst[1] * (1.0 - alpha));
                dst[2] = static_cast<uint8_t>(fillColor.b * 255.0 * alpha + dst[2] * (1.0 - alpha));
                dst[3] = static_cast<uint8_t>(std::min(255.0, dst[3] + fillColor.a * 255.0));
            }
        }
    }
}

void ShapeLayerData::rasterizeGroup(const ShapeGroup& group, PixelBuffer& buffer) const {
    for (const auto& path : group.paths) {
        rasterizePath(path, group.fillColor, group.strokeColor, group.strokeWidth, buffer);
    }
    for (const auto& subGroup : group.groups) {
        rasterizeGroup(subGroup, buffer);
    }
}

void ShapeLayerData::getGroupPaths(const ShapeGroup& group, std::vector<ShapePath>& out) const {
    for (const auto& path : group.paths) {
        out.push_back(path);
    }
    for (const auto& subGroup : group.groups) {
        getGroupPaths(subGroup, out);
    }
}

PixelBuffer ShapeLayerData::rasterize(int width, int height) const {
    PixelBuffer buffer;
    buffer.resize(width, height);

    for (const auto& group : m_contents) {
        rasterizeGroup(group, buffer);
    }

    return buffer;
}

std::vector<ShapePath> ShapeLayerData::getVectorPaths() const {
    std::vector<ShapePath> paths;
    for (const auto& group : m_contents) {
        getGroupPaths(group, paths);
    }
    return paths;
}

} // namespace FreeEffect
