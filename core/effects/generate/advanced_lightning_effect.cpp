#include "../effect_registry.h"
#include "advanced_lightning_effect.h"
#include <cmath>
#include <vector>
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<AdvancedLightningEffect> s_reg("Advanced Lightning", "Generate");

AdvancedLightningEffect::AdvancedLightningEffect() {
    addParameter(EffectParameter::makeVec2("start", "Start", Vec2{0.2, 0.5}));
    addParameter(EffectParameter::makeVec2("end", "End", Vec2{0.8, 0.5}));
    addParameter(EffectParameter::makeFloat("thickness", "Thickness", 1.0, 10.0, 2.0));
    addParameter(EffectParameter::makeFloat("turbulence", "Turbulence", 0.0, 100.0, 30.0));
    addParameter(EffectParameter::makeColor("color", "Color", Color{0.5, 0.6, 1.0, 1.0}));
    addParameter(EffectParameter::makeFloat("glow", "Glow Radius", 0.0, 30.0, 8.0));
    addParameter(EffectParameter::makeInt("branches", "Branches", 0, 15, 5));
    addParameter(EffectParameter::makeFloat("decay", "Branch Decay", 0.1, 1.0, 0.6));
}

std::vector<ParameterGroup> AdvancedLightningEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeVec2("start", "Start", Vec2{0.2, 0.5}),
        EffectParameter::makeVec2("end", "End", Vec2{0.8, 0.5}),
        EffectParameter::makeFloat("thickness", "Thickness", 1.0, 10.0, 2.0),
        EffectParameter::makeFloat("turbulence", "Turbulence", 0.0, 100.0, 30.0),
        EffectParameter::makeColor("color", "Color", Color{0.5, 0.6, 1.0, 1.0}),
        EffectParameter::makeFloat("glow", "Glow Radius", 0.0, 30.0, 8.0),
        EffectParameter::makeInt("branches", "Branches", 0, 15, 5),
        EffectParameter::makeFloat("decay", "Branch Decay", 0.1, 1.0, 0.6)
    }}};
}

std::unique_ptr<Effect> AdvancedLightningEffect::clone() const {
    auto e = std::make_unique<AdvancedLightningEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void AdvancedLightningEffect::render(PixelBuffer& buffer, double time) {
    if (buffer.width == 0 || buffer.height == 0) return;
    Vec2 startP = getVec2Param("start");
    Vec2 endP = getVec2Param("end");
    double thickness = getFloatParam("thickness");
    double turb = getFloatParam("turbulence");
    Color lc = getColorParam("color");
    double glowR = getFloatParam("glow");
    int branches = getIntParam("branches");
    double decay = getFloatParam("decay");
    auto drawSegment = [&](int x0, int y0, int x1, int y1, double thick, double alpha) {
        int dx = x1 - x0;
        int dy = y1 - y0;
        int segs = std::max(2, static_cast<int>(std::sqrt(dx * dx + dy * dy) / 6.0));
        std::vector<int> xs, ys;
        xs.push_back(x0); ys.push_back(y0);
        for (int s = 1; s < segs; s++) {
            double t = static_cast<double>(s) / segs;
            int mx = x0 + static_cast<int>(dx * t);
            int my = y0 + static_cast<int>(dy * t);
            double seed = std::sin(s * 127.1 + x0 * 0.1) * 43758.5453;
            double frac = seed - std::floor(seed);
            mx += static_cast<int>((frac - 0.5) * turb * (1.0 - std::abs(t - 0.5) * 2.0));
            my += static_cast<int>((frac - 0.3) * turb * 0.5 * (1.0 - std::abs(t - 0.5) * 2.0));
            xs.push_back(mx); ys.push_back(my);
        }
        xs.push_back(x1); ys.push_back(y1);
        for (size_t i = 0; i < xs.size() - 1; i++) {
            int steps = std::max(std::abs(xs[i + 1] - xs[i]), std::abs(ys[i + 1] - ys[i])) + 1;
            for (int s = 0; s < steps; s++) {
                double t = steps > 1 ? static_cast<double>(s) / (steps - 1) : 0.0;
                int px = static_cast<int>(xs[i] + (xs[i + 1] - xs[i]) * t);
                int py = static_cast<int>(ys[i] + (ys[i + 1] - ys[i]) * t);
                int rad = static_cast<int>(std::ceil(thick));
                for (int dy2 = -rad; dy2 <= rad; dy2++) {
                    for (int dx2 = -rad; dx2 <= rad; dx2++) {
                        int sx = px + dx2;
                        int sy = py + dy2;
                        if (sx < 0 || sx >= buffer.width || sy < 0 || sy >= buffer.height) continue;
                        double dist = std::sqrt(dx2 * dx2 + dy2 * dy2);
                        if (dist > rad) continue;
                        double falloff = 1.0 - dist / (rad + 0.5);
                        double fa = alpha * falloff;
                        uint8_t* p = buffer.pixelAt(sx, sy);
                        double sa = p[3] / 255.0;
                        double outA = sa + fa * (1.0 - sa);
                        if (outA > 0.001) {
                            p[0] = static_cast<uint8_t>((p[0] * sa + lc.r * 255.0 * fa * (1.0 - sa)) / outA);
                            p[1] = static_cast<uint8_t>((p[1] * sa + lc.g * 255.0 * fa * (1.0 - sa)) / outA);
                            p[2] = static_cast<uint8_t>((p[2] * sa + lc.b * 255.0 * fa * (1.0 - sa)) / outA);
                        }
                        p[3] = static_cast<uint8_t>(std::clamp(outA * 255.0, 0.0, 255.0));
                    }
                }
            }
        }
    };
    int x0 = static_cast<int>(startP.x * buffer.width);
    int y0 = static_cast<int>(startP.y * buffer.height);
    int x1 = static_cast<int>(endP.x * buffer.width);
    int y1 = static_cast<int>(endP.y * buffer.height);
    drawSegment(x0, y0, x1, y1, glowR, lc.a * 0.3);
    drawSegment(x0, y0, x1, y1, thickness, lc.a);
    for (int b = 0; b < branches; b++) {
        double bSeed = std::sin(b * 269.5 + 183.3) * 43758.5453;
        double bFrac = bSeed - std::floor(bSeed);
        double eSeed = std::sin(b * 419.2 + 371.9) * 43758.5453;
        double eFrac = eSeed - std::floor(eSeed);
        int bx0 = x0 + static_cast<int>((x1 - x0) * bFrac);
        int by0 = y0 + static_cast<int>((y1 - y0) * bFrac);
        int bx1 = bx0 + static_cast<int>((eFrac - 0.5) * 120.0);
        int by1 = by0 + static_cast<int>((eFrac - 0.3) * 100.0);
        drawSegment(bx0, by0, bx1, by1, thickness * decay, lc.a * 0.6);
    }
}

} // namespace FreeEffect
