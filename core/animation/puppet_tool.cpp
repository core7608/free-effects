#include "puppet_tool.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

void PuppetMesh::generateMesh(int width, int height) {
    vertices.clear();
    triangles.clear();

    double exp = expansion;
    double minX = -exp;
    double minY = -exp;
    double maxX = width + exp;
    double maxY = height + exp;

    double stepX = (maxX - minX) / resolution;
    double stepY = (maxY - minY) / resolution;

    for (int y = 0; y <= resolution; ++y) {
        for (int x = 0; x <= resolution; ++x) {
            Vec2 v;
            v.x = minX + x * stepX;
            v.y = minY + y * stepY;
            vertices.push_back(v);
        }
    }

    int cols = resolution + 1;
    for (int y = 0; y < resolution; ++y) {
        for (int x = 0; x < resolution; ++x) {
            int i00 = y * cols + x;
            int i10 = y * cols + (x + 1);
            int i01 = (y + 1) * cols + x;
            int i11 = (y + 1) * cols + (x + 1);

            triangles.push_back({i00, i10, i11});
            triangles.push_back({i00, i11, i01});
        }
    }
}

static double crossProduct2D(double ax, double ay, double bx, double by) {
    return ax * by - ay * bx;
}

static bool pointInTriangle(double px, double py,
                             double ax, double ay, double bx, double by, double cx, double cy,
                             double& u, double& v, double& w) {
    double v0x = bx - ax, v0y = by - ay;
    double v1x = cx - ax, v1y = cy - ay;
    double v2x = px - ax, v2y = py - ay;

    double dot00 = v0x * v0x + v0y * v0y;
    double dot01 = v0x * v1x + v0y * v1y;
    double dot11 = v1x * v1x + v1y * v1y;
    double dot20 = v2x * v0x + v2y * v0y;
    double dot21 = v2x * v1x + v2y * v1y;

    double denom = dot00 * dot11 - dot01 * dot01;
    if (std::abs(denom) < 1e-12) return false;

    double invDenom = 1.0 / denom;
    u = (dot11 * dot20 - dot01 * dot21) * invDenom;
    v = (dot00 * dot21 - dot01 * dot20) * invDenom;
    w = 1.0 - u - v;

    return (u >= -1e-6) && (v >= -1e-6) && (w >= -1e-6);
}

void PuppetMesh::deform(const std::vector<PuppetPin>& pins, int width, int height) {
    if (vertices.empty() || pins.empty()) return;

    std::vector<Vec2> original = vertices;

    for (size_t vi = 0; vi < vertices.size(); ++vi) {
        double totalWeight = 0;
        double totalDx = 0;
        double totalDy = 0;

        for (const auto& pin : pins) {
            if (pin.isBase) continue;

            double dx = original[vi].x - pin.x;
            double dy = original[vi].y - pin.y;
            double dist = std::sqrt(dx * dx + dy * dy);

            double radius = std::max(width, height) * 0.5;
            double weight = 1.0 / (1.0 + dist / (radius * 0.3));
            weight = weight * weight;

            totalWeight += weight;
            totalDx += pin.offsetX * weight;
            totalDy += pin.offsetY * weight;
        }

        if (totalWeight > 1e-6) {
            vertices[vi].x = original[vi].x + totalDx / totalWeight;
            vertices[vi].y = original[vi].y + totalDy / totalWeight;
        }
    }
}

void PuppetTool::addPin(double x, double y, bool isBase) {
    PuppetPin pin;
    pin.x = x;
    pin.y = y;
    pin.offsetX = 0;
    pin.offsetY = 0;
    pin.isBase = isBase;
    pin.meshIndex = static_cast<int>(m_pins.size());
    m_pins.push_back(pin);
}

void PuppetTool::removePin(int index) {
    if (index >= 0 && index < static_cast<int>(m_pins.size())) {
        m_pins.erase(m_pins.begin() + index);
    }
}

void PuppetTool::setPinPosition(int index, double x, double y) {
    if (index >= 0 && index < static_cast<int>(m_pins.size())) {
        m_pins[index].x = x;
        m_pins[index].y = y;
    }
}

void PuppetTool::setPinOffset(int index, double offsetX, double offsetY) {
    if (index >= 0 && index < static_cast<int>(m_pins.size())) {
        m_pins[index].offsetX = offsetX;
        m_pins[index].offsetY = offsetY;
    }
}

PuppetMesh PuppetTool::generateMesh(int width, int height) const {
    PuppetMesh mesh;
    mesh.resolution = m_meshResolution;
    mesh.expansion = m_expansion;
    mesh.generateMesh(width, height);
    return mesh;
}

void PuppetTool::applyDeformation(PixelBuffer& target, const PixelBuffer& source,
                                   const PuppetMesh& mesh, int width, int height) const {
    std::vector<Vec2> deformed = mesh.vertices;

    if (!m_pins.empty()) {
        std::vector<PuppetPin> pinCopy = m_pins;
        for (auto& p : pinCopy) {
            p.meshIndex = 0;
        }
        const_cast<PuppetMesh&>(mesh).deform(pinCopy, width, height);
        deformed = mesh.vertices;
    }

    int meshCols = mesh.resolution + 1;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            double px = static_cast<double>(x) + 0.5;
            double py = static_cast<double>(y) + 0.5;

            bool found = false;
            double srcU = 0, srcV = 0;

            for (const auto& tri : mesh.triangles) {
                double ax = deformed[tri[0]].x;
                double ay = deformed[tri[0]].y;
                double bx = deformed[tri[1]].x;
                double by = deformed[tri[1]].y;
                double cx = deformed[tri[2]].x;
                double cy = deformed[tri[2]].y;

                double u, v, w;
                if (pointInTriangle(px, py, ax, ay, bx, by, cx, cy, u, v, w)) {
                    double oAx = mesh.vertices[tri[0]].x;
                    double oAy = mesh.vertices[tri[0]].y;
                    double oBx = mesh.vertices[tri[1]].x;
                    double oBy = mesh.vertices[tri[1]].y;
                    double oCx = mesh.vertices[tri[2]].x;
                    double oCy = mesh.vertices[tri[2]].y;

                    srcU = u * oAx + v * oBx + w * oCx;
                    srcV = u * oAy + v * oBy + w * oCy;
                    found = true;
                    break;
                }
            }

            if (found) {
                int sx = static_cast<int>(srcU);
                int sy = static_cast<int>(srcV);
                sx = std::clamp(sx, 0, source.width - 1);
                sy = std::clamp(sy, 0, source.height - 1);

                const uint8_t* srcPix = source.pixelAt(sx, sy);
                uint8_t* dstPix = target.pixelAt(x, y);
                dstPix[0] = srcPix[0];
                dstPix[1] = srcPix[1];
                dstPix[2] = srcPix[2];
                dstPix[3] = srcPix[3];
            }
        }
    }
}

PixelBuffer PuppetTool::deformBuffer(const PixelBuffer& source, double time) const {
    PixelBuffer result;
    result.resize(source.width, source.height);

    if (m_pins.empty() || source.width == 0 || source.height == 0) {
        result.data = source.data;
        return result;
    }

    PuppetMesh mesh = generateMesh(source.width, source.height);
    applyDeformation(result, source, mesh, source.width, source.height);

    return result;
}

} // namespace FreeEffect
