#include "../effect_registry.h"
#include "lightning_effect.h"
#include <cmath>
#include <vector>
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<LightningEffect> s_reg("Lightning", "Simulation");

LightningEffect::LightningEffect() {
    addParameter(EffectParameter::makeVec2("start", "Start Point", Vec2{0.5, 0.0}));
    addParameter(EffectParameter::makeVec2("end", "End Point", Vec2{0.5, 1.0}));
    addParameter(EffectParameter::makeFloat("thickness", "Thickness", 1.0, 10.0, 2.0));
    addParameter(EffectParameter::makeFloat("turbulence", "Turbulence", 0.0, 50.0, 15.0));
    addParameter(EffectParameter::makeInt("branches", "Branches", 0, 10, 3));
    addParameter(EffectParameter::makeColor("color", "Color", Color{0.5, 0.6, 1.0, 1.0}));
    addParameter(EffectParameter::makeFloat("glow", "Glow", 0.0, 20.0, 5.0));
}

std::vector<ParameterGroup> LightningEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeVec2("start", "Start Point", Vec2{0.5, 0.0}),
        EffectParameter::makeVec2("end", "End Point", Vec2{0.5, 1.0}),
        EffectParameter::makeFloat("thickness", "Thickness", 1.0, 10.0, 2.0),
        EffectParameter::makeFloat("turbulence", "Turbulence", 0.0, 50.0, 15.0),
        EffectParameter::makeInt("branches", "Branches", 0, 10, 3),
        EffectParameter::makeColor("color", "Color", Color{0.5, 0.6, 1.0, 1.0}),
        EffectParameter::makeFloat("glow", "Glow", 0.0, 20.0, 5.0)
    }}};
}

std::unique_ptr<Effect> LightningEffect::clone() const {
    auto e = std::make_unique<LightningEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void LightningEffect::drawBolt(PixelBuffer& buffer, int x0, int y0, int x1, int y1,
                                double thickness, double r, double g, double b, double a, int depth) {
    if (depth <= 0 || thickness < 0.3) return;
    int dx = x1 - x0;
    int dy = y1 - y0;
    int segments = std::max(2, static_cast<int>(std::sqrt(dx * dx + dy * dy) / 8.0));
    double turb = getFloatParam("turbulence");
    std::vector<int> xs, ys;
    xs.push_back(x0);
    ys.push_back(y0);
    for (int s = 1; s < segments; s++) {
        double t = static_cast<double>(s) / segments;
        int mx = x0 + static_cast<int>(dx * t);
        int my = y0 + static_cast<int>(dy * t);
        double seed = std::sin(s * 127.1 + depth * 311.7) * 43758.5453;
        double frac = seed - std::floor(seed);
        double offset = (frac - 0.5) * turb * 2.0;
        mx += static_cast<int>(offset * (1.0 - std::abs(t - 0.5) * 2.0));
        my += static_cast<int>(offset * 0.5 * (1.0 - std::abs(t - 0.5) * 2.0));
        xs.push_back(mx);
        ys.push_back(my);
    }
    xs.push_back(x1);
    ys.push_back(y1);
    for (size_t i = 0; i < xs.size() - 1; i++) {
        int lx0 = xs[i], ly0 = ys[i];
        int lx1 = xs[i + 1], ly1 = ys[i + 1];
        int steps = std::max(std::abs(lx1 - lx0), std::abs(ly1 - ly0)) + 1;
        for (int s = 0; s < steps; s++) {
            double t = steps > 1 ? static_cast<double>(s) / (steps - 1) : 0.0;
            int px = static_cast<int>(lx0 + (lx1 - lx0) * t);
            int py = static_cast<int>(ly0 + (ly1 - ly0) * t);
            int rad = static_cast<int>(std::ceil(thickness));
            for (int dy2 = -rad; dy2 <= rad; dy2++) {
                for (int dx2 = -rad; dx2 <= rad; dx2++) {
                    int sx = px + dx2;
                    int sy = py + dy2;
                    if (sx < 0 || sx >= buffer.width || sy < 0 || sy >= buffer.height) continue;
                    double dist = std::sqrt(dx2 * dx2 + dy2 * dy2);
                    if (dist > rad) continue;
                    double falloff = 1.0 - dist / (rad + 0.5);
                    double fa = a * falloff;
                    uint8_t* p = buffer.pixelAt(sx, sy);
                    double sa = p[3] / 255.0;
                    double outA = sa + fa * (1.0 - sa);
                    if (outA > 0.001) {
                        p[0] = static_cast<uint8_t>(std::clamp((p[0] * sa + r * fa * (1.0 - sa)) / outA, 0.0, 255.0));
                        p[1] = static_cast<uint8_t>(std::clamp((p[1] * sa + g * fa * (1.0 - sa)) / outA, 0.0, 255.0));
                        p[2] = static_cast<uint8_t>(std::clamp((p[2] * sa + b * fa * (1.0 - sa)) / outA, 0.0, 255.0));
                    }
                    p[3] = static_cast<uint8_t>(std::clamp(outA * 255.0, 0.0, 255.0));
                }
            }
        }
    }
}

void LightningEffect::render(PixelBuffer& buffer, double time) {
    if (buffer.width == 0 || buffer.height == 0) return;
    Vec2 startP = getVec2Param("start");
    Vec2 endP = getVec2Param("end");
    double thickness = getFloatParam("thickness");
    int branches = getIntParam("branches");
    Color lc = getColorParam("color");
    double glow = getFloatParam("glow");
    int x0 = static_cast<int>(startP.x * buffer.width);
    int y0 = static_cast<int>(startP.y * buffer.height);
    int x1 = static_cast<int>(endP.x * buffer.width);
    int y1 = static_cast<int>(endP.y * buffer.height);
    double cr = lc.r * 255.0;
    double cg = lc.g * 255.0;
    double cb = lc.b * 255.0;
    drawBolt(buffer, x0, y0, x1, y1, glow, cr * 0.3, cg * 0.3, cb * 0.3, lc.a * 0.3, 1);
    drawBolt(buffer, x0, y0, x1, y1, thickness, cr, cg, cb, lc.a, 2);
    for (int b = 0; b < branches; b++) {
        double bSeed = std::sin(b * 127.1 + 311.7) * 43758.5453;
        double bFrac = bSeed - std::floor(bSeed);
        int bx0 = x0 + static_cast<int>((x1 - x0) * bFrac);
        int by0 = y0 + static_cast<int>((y1 - y0) * bFrac);
        double eSeed = std::sin(b * 269.5 + 183.3) * 43758.5453;
        double eFrac = eSeed - std::floor(eSeed);
        int bx1 = bx0 + static_cast<int>((eFrac - 0.5) * 100.0);
        int by1 = by0 + static_cast<int>((eFrac - 0.3) * 80.0);
        drawBolt(buffer, bx0, by0, bx1, by1, thickness * 0.5, cr, cg, cb, lc.a * 0.6, 1);
    }
}

} // namespace FreeEffect
