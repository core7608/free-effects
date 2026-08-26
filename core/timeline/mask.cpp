#include "mask.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

Mask::Mask()
    : m_id(generateUUID())
    , m_opacity("Mask Opacity") {
    m_opacity.setDefaultValue(100.0);
}

Mask::Mask(const std::string& name, MaskMode mode)
    : m_id(generateUUID())
    , m_name(name)
    , m_mode(mode)
    , m_opacity("Mask Opacity") {
    m_opacity.setDefaultValue(100.0);
}

void Mask::addVertex(const MaskVertex& vertex) {
    m_vertices.push_back(vertex);
}

void Mask::removeVertex(int index) {
    if (index >= 0 && index < static_cast<int>(m_vertices.size())) {
        m_vertices.erase(m_vertices.begin() + index);
    }
}

void Mask::setVertexPosition(int index, const Vec2& position) {
    if (index >= 0 && index < static_cast<int>(m_vertices.size())) {
        m_vertices[index].position = position;
    }
}

bool Mask::pointOnCurveSegment(double px, double py,
                                const Vec2& p0, const BezierHandle& h0,
                                const Vec2& p1, const BezierHandle& h1) const {
    Vec2 cp1 = {p0.x + h0.outX, p0.y + h0.outY};
    Vec2 cp2 = {p1.x + h1.inX, p1.y + h1.inY};

    double minDist = 1e10;
    for (int i = 1; i <= 20; ++i) {
        double t = static_cast<double>(i) / 20.0;
        double u = 1.0 - t;

        double bx = u * u * u * p0.x + 3.0 * u * u * t * cp1.x +
                    3.0 * u * t * t * cp2.x + t * t * t * p1.x;
        double by = u * u * u * p0.y + 3.0 * u * u * t * cp1.y +
                    3.0 * u * t * t * cp2.y + t * t * t * p1.y;

        double dx = px - bx;
        double dy = py - by;
        double dist = dx * dx + dy * dy;
        minDist = std::min(minDist, dist);
    }

    return minDist < 16.0;
}

bool Mask::isPointOnBoundary(double x, double y, double tolerance) const {
    int n = static_cast<int>(m_vertices.size());
    if (n < 2) return false;

    for (int i = 0; i < n; ++i) {
        int next = (i + 1) % n;
        const Vec2& p0 = m_vertices[i].position;
        const Vec2& p1 = m_vertices[next].position;

        Vec2 cp1 = {p0.x + m_vertices[i].outHandle.outX,
                     p0.y + m_vertices[i].outHandle.outY};
        Vec2 cp2 = {p1.x + m_vertices[next].inHandle.inX,
                     p1.y + m_vertices[next].inHandle.inY};

        bool isLinear = (std::abs(m_vertices[i].outHandle.outX) < 1e-6 &&
                         std::abs(m_vertices[i].outHandle.outY) < 1e-6 &&
                         std::abs(m_vertices[next].inHandle.inX) < 1e-6 &&
                         std::abs(m_vertices[next].inHandle.inY) < 1e-6);

        if (isLinear) {
            double dx = p1.x - p0.x;
            double dy = p1.y - p0.y;
            double lenSq = dx * dx + dy * dy;
            if (lenSq < 1e-10) continue;

            double t = std::clamp(((x - p0.x) * dx + (y - p0.y) * dy) / lenSq, 0.0, 1.0);
            double projX = p0.x + t * dx;
            double projY = p0.y + t * dy;
            double dist = std::sqrt((x - projX) * (x - projX) + (y - projY) * (y - projY));

            if (dist <= tolerance) return true;
        } else {
            for (int s = 1; s <= 20; ++s) {
                double t = static_cast<double>(s) / 20.0;
                double u = 1.0 - t;

                double bx = u * u * u * p0.x + 3.0 * u * u * t * cp1.x +
                            3.0 * u * t * t * cp2.x + t * t * t * p1.x;
                double by = u * u * u * p0.y + 3.0 * u * u * t * cp1.y +
                            3.0 * u * t * t * cp2.y + t * t * t * p1.y;

                double dist = std::sqrt((x - bx) * (x - bx) + (y - by) * (y - by));
                if (dist <= tolerance) return true;
            }
        }
    }
    return false;
}

bool Mask::isPointInside(double x, double y) const {
    int n = static_cast<int>(m_vertices.size());
    if (n < 3) return false;

    if (isPointOnBoundary(x, y)) return true;

    int crossings = 0;
    for (int i = 0; i < n; ++i) {
        int next = (i + 1) % n;
        const Vec2& p0 = m_vertices[i].position;
        const Vec2& p1 = m_vertices[next].position;

        bool hasBezier = (std::abs(m_vertices[i].outHandle.outX) > 1e-6 ||
                          std::abs(m_vertices[i].outHandle.outY) > 1e-6 ||
                          std::abs(m_vertices[next].inHandle.inX) > 1e-6 ||
                          std::abs(m_vertices[next].inHandle.inY) > 1e-6);

        if (hasBezier) {
            Vec2 cp1 = {p0.x + m_vertices[i].outHandle.outX,
                         p0.y + m_vertices[i].outHandle.outY};
            Vec2 cp2 = {p1.x + m_vertices[next].inHandle.inX,
                         p1.y + m_vertices[next].inHandle.inY};

            Vec2 prevPt = p0;
            for (int s = 1; s <= 10; ++s) {
                double t = static_cast<double>(s) / 10.0;
                double u = 1.0 - t;

                double bx = u * u * u * p0.x + 3.0 * u * u * t * cp1.x +
                            3.0 * u * t * t * cp2.x + t * t * t * p1.x;
                double by = u * u * u * p0.y + 3.0 * u * u * t * cp1.y +
                            3.0 * u * t * t * cp2.y + t * t * t * p1.y;

                Vec2 currPt = {bx, by};

                if ((prevPt.y > y) != (currPt.y > y)) {
                    double xIntersect = prevPt.x + (currPt.x - prevPt.x) *
                                        (y - prevPt.y) / (currPt.y - prevPt.y);
                    if (x < xIntersect) {
                        crossings++;
                    }
                }
                prevPt = currPt;
            }
        } else {
            if ((p0.y > y) != (p1.y > y)) {
                double xIntersect = p0.x + (p1.x - p0.x) * (y - p0.y) / (p1.y - p0.y);
                if (x < xIntersect) {
                    crossings++;
                }
            }
        }
    }

    bool inside = (crossings % 2) == 1;
    if (m_inverted) inside = !inside;
    return inside;
}

PixelBuffer Mask::applyMask(const PixelBuffer& input) const {
    PixelBuffer output;
    output.resize(input.width, input.height);

    for (int y = 0; y < input.height; ++y) {
        for (int x = 0; x < input.width; ++x) {
            const uint8_t* src = input.pixelAt(x, y);
            uint8_t* dst = output.pixelAt(x, y);

            bool inside = isPointInside(static_cast<double>(x), static_cast<double>(y));

            if (inside) {
                double opacity = m_opacity.getValueAtTime(0.0) / 100.0;
                dst[0] = static_cast<uint8_t>(src[0] * opacity);
                dst[1] = static_cast<uint8_t>(src[1] * opacity);
                dst[2] = static_cast<uint8_t>(src[2] * opacity);
                dst[3] = static_cast<uint8_t>(src[3] * opacity);
            } else {
                dst[0] = 0;
                dst[1] = 0;
                dst[2] = 0;
                dst[3] = 0;
            }
        }
    }

    return output;
}

} // namespace FreeEffect
