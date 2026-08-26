#include "freehand_tool.h"
#include <cmath>
#include <algorithm>

namespace FreeEffect {

void FreehandTool::onMouseDown(double x, double y, int modifiers) {
    m_rawPoints.clear();
    m_simplifiedPath.clear();
    m_drawing = true;
    m_rawPoints.push_back({x, y});
}

void FreehandTool::onMouseMove(double x, double y, int modifiers) {
    if (!m_drawing) return;

    const auto& last = m_rawPoints.back();
    double dx = x - last.x;
    double dy = y - last.y;
    double dist = std::sqrt(dx * dx + dy * dy);

    if (dist >= 2.0) {
        m_rawPoints.push_back({x, y});
    }
}

void FreehandTool::onMouseUp(double x, double y, int modifiers) {
    if (!m_drawing) return;
    m_drawing = false;

    if (m_rawPoints.size() < 2) {
        m_rawPoints.push_back({x, y});
    }

    std::vector<Point2D> simplified;
    rdpSimplify(m_rawPoints, m_simplifyTolerance, simplified);

    m_simplifiedPath.clear();
    for (const auto& pt : simplified) {
        BezierPoint bp;
        bp.position = pt;
        bp.handleIn = pt;
        bp.handleOut = pt;
        bp.smooth = true;
        m_simplifiedPath.push_back(bp);
    }

    if (m_simplifiedPath.size() >= 2) {
        for (size_t i = 1; i + 1 < m_simplifiedPath.size(); i++) {
            const auto& prev = m_simplifiedPath[i - 1].position;
            const auto& curr = m_simplifiedPath[i].position;
            const auto& next = m_simplifiedPath[i + 1].position;

            double dx1 = curr.x - prev.x;
            double dy1 = curr.y - prev.y;
            double dx2 = next.x - curr.x;
            double dy2 = next.y - curr.y;

            double len1 = std::sqrt(dx1 * dx1 + dy1 * dy1);
            double len2 = std::sqrt(dx2 * dx2 + dy2 * dy2);

            if (len1 > 1e-6 && len2 > 1e-6) {
                double t = len1 / (len1 + len2);
                m_simplifiedPath[i].handleIn = {
                    curr.x - dx2 * t * 0.25,
                    curr.y - dy2 * t * 0.25};
                m_simplifiedPath[i].handleOut = {
                    curr.x + dx1 * (1.0 - t) * 0.25,
                    curr.y + dy1 * (1.0 - t) * 0.25};
            }
        }
    }
}

ToolResult FreehandTool::getResult() const {
    ToolResult result;
    result.path = m_simplifiedPath;
    result.consumed = m_drawing || !m_simplifiedPath.empty();
    result.finished = !m_drawing && !m_simplifiedPath.empty();
    return result;
}

void FreehandTool::reset() {
    m_rawPoints.clear();
    m_simplifiedPath.clear();
    m_drawing = false;
}

double FreehandTool::pointToLineDistance(
    const Point2D& p, const Point2D& a, const Point2D& b) {

    double dx = b.x - a.x;
    double dy = b.y - a.y;
    double lenSq = dx * dx + dy * dy;

    if (lenSq < 1e-12) {
        double ex = p.x - a.x;
        double ey = p.y - a.y;
        return std::sqrt(ex * ex + ey * ey);
    }

    double t = std::clamp(((p.x - a.x) * dx + (p.y - a.y) * dy) / lenSq, 0.0, 1.0);
    double projX = a.x + t * dx;
    double projY = a.y + t * dy;
    double ex = p.x - projX;
    double ey = p.y - projY;
    return std::sqrt(ex * ex + ey * ey);
}

void FreehandTool::rdpSimplify(
    const std::vector<Point2D>& points, double epsilon,
    std::vector<Point2D>& result) {

    if (points.size() <= 2) {
        result = points;
        return;
    }

    double maxDist = 0.0;
    size_t maxIndex = 0;

    for (size_t i = 1; i + 1 < points.size(); i++) {
        double dist = pointToLineDistance(points[i], points.front(), points.back());
        if (dist > maxDist) {
            maxDist = dist;
            maxIndex = i;
        }
    }

    if (maxDist > epsilon) {
        std::vector<Point2D> left(points.begin(), points.begin() + maxIndex + 1);
        std::vector<Point2D> right(points.begin() + maxIndex, points.end());

        std::vector<Point2D> leftResult, rightResult;
        rdpSimplify(left, epsilon, leftResult);
        rdpSimplify(right, epsilon, rightResult);

        result = leftResult;
        result.insert(result.end(), rightResult.begin() + 1, rightResult.end());
    } else {
        result.push_back(points.front());
        result.push_back(points.back());
    }
}

} // namespace FreeEffect
