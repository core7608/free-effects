#include "../effect_registry.h"
#include "plexus_effect.h"
#include <cmath>
#include <vector>
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<PlexusEffect> s_reg("Plexus", "Plugin Effect", "Triptych");

PlexusEffect::PlexusEffect() {
    addParameter(EffectParameter::makeInt("point_count", "Point Count", 10, 500, 100));
    addParameter(EffectParameter::makeFloat("connection_distance", "Connection Distance", 10.0, 300.0, 80.0));
    addParameter(EffectParameter::makeFloat("speed", "Speed", 0.0, 3.0, 0.5));
    addParameter(EffectParameter::makeColor("color", "Color", Color{0.0, 0.8, 1.0, 0.8}));
    addParameter(EffectParameter::makeFloat("point_size", "Point Size", 1.0, 10.0, 3.0));
    addParameter(EffectParameter::makeFloat("line_thickness", "Line Thickness", 0.5, 5.0, 1.0));
}

std::vector<ParameterGroup> PlexusEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeInt("point_count", "Point Count", 10, 500, 100),
        EffectParameter::makeFloat("connection_distance", "Connection Distance", 10.0, 300.0, 80.0),
        EffectParameter::makeFloat("speed", "Speed", 0.0, 3.0, 0.5),
        EffectParameter::makeColor("color", "Color", Color{0.0, 0.8, 1.0, 0.8}),
        EffectParameter::makeFloat("point_size", "Point Size", 1.0, 10.0, 3.0),
        EffectParameter::makeFloat("line_thickness", "Line Thickness", 0.5, 5.0, 1.0)
    }}};
}

std::unique_ptr<Effect> PlexusEffect::clone() const {
    auto e = std::make_unique<PlexusEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void PlexusEffect::render(PixelBuffer& buffer, double time) {
    if (buffer.width == 0 || buffer.height == 0) return;
    int pointCount = getIntParam("point_count");
    double connDist = getFloatParam("connection_distance");
    double spd = getFloatParam("speed");
    Color pc = getColorParam("color");
    double ptSize = getFloatParam("point_size");
    double lineThick = getFloatParam("line_thickness");
    struct Point2D { double x, y; };
    std::vector<Point2D> points(pointCount);
    for (int i = 0; i < pointCount; i++) {
        double seed1 = std::sin(i * 127.1 + 311.7) * 43758.5453;
        double seed2 = std::sin(i * 269.5 + 183.3) * 43758.5453;
        double seed3 = std::sin(i * 419.2 + 371.9) * 43758.5453;
        double seed4 = std::sin(i * 573.1 + 193.3) * 43758.5453;
        double f1 = seed1 - std::floor(seed1);
        double f2 = seed2 - std::floor(seed2);
        double f3 = seed3 - std::floor(seed3);
        double f4 = seed4 - std::floor(seed4);
        points[i].x = f1 * buffer.width + std::sin(time * spd + f3 * 6.28) * 30.0;
        points[i].y = f2 * buffer.height + std::cos(time * spd + f4 * 6.28) * 30.0;
    }
    double cr = pc.r * 255.0;
    double cg = pc.g * 255.0;
    double cb = pc.b * 255.0;
    for (int i = 0; i < pointCount; i++) {
        for (int j = i + 1; j < pointCount; j++) {
            double dx = points[i].x - points[j].x;
            double dy = points[i].y - points[j].y;
            double dist = std::sqrt(dx * dx + dy * dy);
            if (dist > connDist) continue;
            double alpha = pc.a * (1.0 - dist / connDist);
            int x0 = static_cast<int>(points[i].x);
            int y0 = static_cast<int>(points[i].y);
            int x1 = static_cast<int>(points[j].x);
            int y1 = static_cast<int>(points[j].y);
            int steps = std::max(std::abs(x1 - x0), std::abs(y1 - y0)) + 1;
            int lt = static_cast<int>(std::ceil(lineThick));
            for (int s = 0; s < steps; s++) {
                double t = steps > 1 ? static_cast<double>(s) / (steps - 1) : 0.0;
                int px = static_cast<int>(x0 + (x1 - x0) * t);
                int py = static_cast<int>(y0 + (y1 - y0) * t);
                for (int dy2 = -lt / 2; dy2 <= lt / 2; dy2++) {
                    for (int dx2 = -lt / 2; dx2 <= lt / 2; dx2++) {
                        int sx = px + dx2;
                        int sy = py + dy2;
                        if (sx < 0 || sx >= buffer.width || sy < 0 || sy >= buffer.height) continue;
                        uint8_t* p = buffer.pixelAt(sx, sy);
                        double sa = p[3] / 255.0;
                        double outA = sa + alpha * (1.0 - sa);
                        if (outA > 0.001) {
                            p[0] = static_cast<uint8_t>((p[0] * sa + cr * alpha * (1.0 - sa)) / outA);
                            p[1] = static_cast<uint8_t>((p[1] * sa + cg * alpha * (1.0 - sa)) / outA);
                            p[2] = static_cast<uint8_t>((p[2] * sa + cb * alpha * (1.0 - sa)) / outA);
                        }
                        p[3] = static_cast<uint8_t>(std::clamp(outA * 255.0, 0.0, 255.0));
                    }
                }
            }
        }
    }
    int ir = static_cast<int>(std::ceil(ptSize));
    for (int i = 0; i < pointCount; i++) {
        int ipx = static_cast<int>(points[i].x);
        int ipy = static_cast<int>(points[i].y);
        for (int dy = -ir; dy <= ir; dy++) {
            for (int dx = -ir; dx <= ir; dx++) {
                int sx = ipx + dx;
                int sy = ipy + dy;
                if (sx < 0 || sx >= buffer.width || sy < 0 || sy >= buffer.height) continue;
                double dist = std::sqrt(dx * dx + dy * dy);
                if (dist > ir) continue;
                double falloff = 1.0 - dist / (ir + 0.5);
                double fa = pc.a * falloff;
                uint8_t* p = buffer.pixelAt(sx, sy);
                double sa = p[3] / 255.0;
                double outA = sa + fa * (1.0 - sa);
                if (outA > 0.001) {
                    p[0] = static_cast<uint8_t>((p[0] * sa + cr * fa * (1.0 - sa)) / outA);
                    p[1] = static_cast<uint8_t>((p[1] * sa + cg * fa * (1.0 - sa)) / outA);
                    p[2] = static_cast<uint8_t>((p[2] * sa + cb * fa * (1.0 - sa)) / outA);
                }
                p[3] = static_cast<uint8_t>(std::clamp(outA * 255.0, 0.0, 255.0));
            }
        }
    }
}

} // namespace FreeEffect
