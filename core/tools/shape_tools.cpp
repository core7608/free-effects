#include "shape_tools.h"
#include <cmath>
#include <algorithm>

namespace FreeEffect {

static constexpr double kPi = 3.14159265358979323846;

void ShapeToolBase::onMouseDown(double x, double y, int modifiers) {
    m_start = {x, y};
    m_end = {x, y};
    m_drawing = true;
    m_constrain = (modifiers & 0x02000000) != 0;
    m_centerOut = (modifiers & 0x04000000) != 0;
}

void ShapeToolBase::onMouseMove(double x, double y, int modifiers) {
    if (!m_drawing) return;
    m_end = {x, y};
    m_constrain = (modifiers & 0x02000000) != 0;
    m_centerOut = (modifiers & 0x04000000) != 0;

    double dx = m_end.x - m_start.x;
    double dy = m_end.y - m_start.y;

    if (m_constrain) {
        double maxDim = std::max(std::abs(dx), std::abs(dy));
        dx = (dx >= 0 ? maxDim : -maxDim);
        dy = (dy >= 0 ? maxDim : -maxDim);
        m_end = {m_start.x + dx, m_start.y + dy};
    }

    double x1, y1, x2, y2;
    if (m_centerOut) {
        x1 = m_start.x - dx;
        y1 = m_start.y - dy;
        x2 = m_start.x + dx;
        y2 = m_start.y + dy;
    } else {
        x1 = m_start.x;
        y1 = m_start.y;
        x2 = m_end.x;
        y2 = m_end.y;
    }

    if (x1 > x2) std::swap(x1, x2);
    if (y1 > y2) std::swap(y1, y2);

    m_currentPath = buildShapePoints(x1, y1, x2, y2, m_polygonSides);
}

void ShapeToolBase::onMouseUp(double x, double y, int modifiers) {
    m_drawing = false;

    double dx = m_end.x - m_start.x;
    double dy = m_end.y - m_start.y;
    if (std::abs(dx) < 1.0 && std::abs(dy) < 1.0) {
        double half = 50.0;
        m_currentPath = buildShapePoints(
            m_start.x - half, m_start.y - half,
            m_start.x + half, m_start.y + half,
            m_polygonSides);
    }
}

ToolResult ShapeToolBase::getResult() const {
    ToolResult result;
    result.path = m_currentPath;
    result.consumed = m_drawing || !m_currentPath.empty();
    result.finished = !m_drawing && !m_currentPath.empty();
    return result;
}

void ShapeToolBase::reset() {
    m_drawing = false;
    m_currentPath.clear();
}

std::vector<BezierPoint> RectangleTool::buildShapePoints(
    double x1, double y1, double x2, double y2, int /*sides*/) const {

    std::vector<BezierPoint> pts;
    pts.resize(4);

    double w = (x2 - x1) * 0.5522847498;
    double h = (y2 - y1) * 0.5522847498;

    pts[0].position = {x1, y1};
    pts[0].handleIn  = {x1, y1 - h};
    pts[0].handleOut = {x1, y1 + h};

    pts[1].position = {x1, y2};
    pts[1].handleIn  = {x1 - w, y2};
    pts[1].handleOut = {x1 + w, y2};

    pts[2].position = {x2, y2};
    pts[2].handleIn  = {x2, y2 + h};
    pts[2].handleOut = {x2, y2 - h};

    pts[3].position = {x2, y1};
    pts[3].handleIn  = {x2 + w, y1};
    pts[3].handleOut = {x2 - w, y1};

    return pts;
}

std::vector<BezierPoint> EllipseTool::buildShapePoints(
    double x1, double y1, double x2, double y2, int /*sides*/) const {

    std::vector<BezierPoint> pts;
    pts.resize(4);

    double cx = (x1 + x2) * 0.5;
    double cy = (y1 + y2) * 0.5;
    double rx = (x2 - x1) * 0.5;
    double ry = (y2 - y1) * 0.5;

    double k = 0.5522847498;
    double kx = rx * k;
    double ky = ry * k;

    pts[0].position = {cx, cy - ry};
    pts[0].handleIn  = {cx - kx, cy - ry};
    pts[0].handleOut = {cx + kx, cy - ry};

    pts[1].position = {cx + rx, cy};
    pts[1].handleIn  = {cx + rx, cy - ky};
    pts[1].handleOut = {cx + rx, cy + ky};

    pts[2].position = {cx, cy + ry};
    pts[2].handleIn  = {cx + kx, cy + ry};
    pts[2].handleOut = {cx - kx, cy + ry};

    pts[3].position = {cx - rx, cy};
    pts[3].handleIn  = {cx - rx, cy + ky};
    pts[3].handleOut = {cx - rx, cy - ky};

    return pts;
}

std::vector<BezierPoint> PolygonTool::buildShapePoints(
    double x1, double y1, double x2, double y2, int sides) const {

    std::vector<BezierPoint> pts;
    if (sides < 3) sides = 3;

    double cx = (x1 + x2) * 0.5;
    double cy = (y1 + y2) * 0.5;
    double rx = (x2 - x1) * 0.5;
    double ry = (y2 - y1) * 0.5;
    double radius = std::min(rx, ry);

    double startAngle = -kPi / 2.0;

    pts.resize(sides);
    for (int i = 0; i < sides; i++) {
        double angle = startAngle + (2.0 * kPi * i) / sides;
        pts[i].position = {cx + std::cos(angle) * radius,
                           cy + std::sin(angle) * radius};
        pts[i].handleIn = pts[i].position;
        pts[i].handleOut = pts[i].position;
        pts[i].smooth = false;
    }
    return pts;
}

void PolygonTool::onKeyDown(int key) {
    if (key == 0x01000015) {
        m_polygonSides = std::min(m_polygonSides + 1, 36);
    } else if (key == 0x01000014) {
        m_polygonSides = std::max(m_polygonSides - 1, 3);
    }
}

std::vector<BezierPoint> StarTool::buildShapePoints(
    double x1, double y1, double x2, double y2, int sides) const {

    std::vector<BezierPoint> pts;
    if (sides < 3) sides = 5;

    double cx = (x1 + x2) * 0.5;
    double cy = (y1 + y2) * 0.5;
    double rx = (x2 - x1) * 0.5;
    double ry = (y2 - y1) * 0.5;
    double outerRadius = std::min(rx, ry);
    double innerRadius = outerRadius * 0.4;

    double startAngle = -kPi / 2.0;
    int numPoints = sides * 2;

    pts.resize(numPoints);
    for (int i = 0; i < numPoints; i++) {
        double angle = startAngle + (2.0 * kPi * i) / numPoints;
        double r = (i % 2 == 0) ? outerRadius : innerRadius;
        pts[i].position = {cx + std::cos(angle) * r,
                           cy + std::sin(angle) * r};
        pts[i].handleIn = pts[i].position;
        pts[i].handleOut = pts[i].position;
        pts[i].smooth = false;
    }
    return pts;
}

void StarTool::onKeyDown(int key) {
    if (key == 0x01000015) {
        m_polygonSides = std::min(m_polygonSides + 1, 36);
    } else if (key == 0x01000014) {
        m_polygonSides = std::max(m_polygonSides - 1, 3);
    }
}

} // namespace FreeEffect
