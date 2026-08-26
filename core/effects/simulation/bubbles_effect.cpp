#include "../effect_registry.h"
#include "bubbles_effect.h"
#include <cmath>
#include <vector>
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<BubblesEffect> s_reg("Bubbles", "Simulation");

BubblesEffect::BubblesEffect() {
    addParameter(EffectParameter::makeFloat("intensity", "Intensity", 0.0, 1.0, 0.5));
    addParameter(EffectParameter::makeInt("count", "Bubble Count", 5, 500, 100));
    addParameter(EffectParameter::makeFloat("speed", "Rise Speed", 0.1, 3.0, 0.8));
    addParameter(EffectParameter::makeFloat("size", "Bubble Size", 2.0, 30.0, 8.0));
    addParameter(EffectParameter::makeColor("color", "Bubble Color", Color{0.6, 0.8, 1.0, 0.3}));
    addParameter(EffectParameter::makeFloat("wobble", "Wobble", 0.0, 5.0, 1.5));
}

std::vector<ParameterGroup> BubblesEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeFloat("intensity", "Intensity", 0.0, 1.0, 0.5),
        EffectParameter::makeInt("count", "Bubble Count", 5, 500, 100),
        EffectParameter::makeFloat("speed", "Rise Speed", 0.1, 3.0, 0.8),
        EffectParameter::makeFloat("size", "Bubble Size", 2.0, 30.0, 8.0),
        EffectParameter::makeColor("color", "Bubble Color", Color{0.6, 0.8, 1.0, 0.3}),
        EffectParameter::makeFloat("wobble", "Wobble", 0.0, 5.0, 1.5)
    }}};
}

std::unique_ptr<Effect> BubblesEffect::clone() const {
    auto e = std::make_unique<BubblesEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void BubblesEffect::render(PixelBuffer& buffer, double time) {
    if (buffer.width == 0 || buffer.height == 0) return;
    double intensity = getFloatParam("intensity");
    int count = getIntParam("count");
    double spd = getFloatParam("speed");
    double sz = getFloatParam("size");
    Color bc = getColorParam("color");
    double wobbleAmp = getFloatParam("wobble");
    int effective = static_cast<int>(count * intensity);
    double cr = bc.r * 255.0;
    double cg = bc.g * 255.0;
    double cb = bc.b * 255.0;
    for (int i = 0; i < effective; i++) {
        double seed1 = std::sin(i * 127.1 + 311.7) * 43758.5453;
        double seed2 = std::sin(i * 269.5 + 183.3) * 43758.5453;
        double seed3 = std::sin(i * 419.2 + 371.9) * 43758.5453;
        double f1 = seed1 - std::floor(seed1);
        double f2 = seed2 - std::floor(seed2);
        double f3 = seed3 - std::floor(seed3);
        double baseX = f1 * buffer.width;
        double riseSpeed = (0.5 + f2 * 0.5) * spd * 80.0;
        double yPos = buffer.height - std::fmod(time * riseSpeed + f3 * buffer.height * 2.0, buffer.height + 60.0) + 30.0;
        double xWobble = std::sin(time * 2.0 + i * 1.7) * wobbleAmp * 10.0;
        double xPos = baseX + xWobble;
        xPos = std::fmod(xPos + buffer.width * 2.0, static_cast<double>(buffer.width));
        double radius = sz * (0.3 + f1 * 0.7);
        int ir = static_cast<int>(std::ceil(radius));
        int ipx = static_cast<int>(std::round(xPos));
        int ipy = static_cast<int>(std::round(yPos));
        for (int dy = -ir; dy <= ir; dy++) {
            for (int dx = -ir; dx <= ir; dx++) {
                int sx = ipx + dx;
                int sy = ipy + dy;
                if (sx < 0 || sx >= buffer.width || sy < 0 || sy >= buffer.height) continue;
                double dist = std::sqrt(dx * dx + dy * dy);
                if (dist > radius) continue;
                double edgeDist = radius - dist;
                double edgeFactor = std::min(1.0, edgeDist / (radius * 0.3));
                double rimFactor = 1.0 - (dist / radius);
                rimFactor = rimFactor * rimFactor;
                double fa = bc.a * rimFactor * edgeFactor;
                if (fa < 0.01) continue;
                double highlight = 0.0;
                double hx = dx / radius - 0.3;
                double hy = dy / radius - 0.3;
                double hDist = std::sqrt(hx * hx + hy * hy);
                if (hDist < 0.4) highlight = (1.0 - hDist / 0.4) * 0.5;
                double pr = cr * rimFactor + 255.0 * highlight;
                double pg = cg * rimFactor + 255.0 * highlight;
                double pb = cb * rimFactor + 255.0 * highlight;
                uint8_t* p = buffer.pixelAt(sx, sy);
                double sa = p[3] / 255.0;
                double outA = sa + fa * (1.0 - sa);
                if (outA > 0.001) {
                    p[0] = static_cast<uint8_t>(std::clamp((p[0] * sa + pr * fa * (1.0 - sa)) / outA, 0.0, 255.0));
                    p[1] = static_cast<uint8_t>(std::clamp((p[1] * sa + pg * fa * (1.0 - sa)) / outA, 0.0, 255.0));
                    p[2] = static_cast<uint8_t>(std::clamp((p[2] * sa + pb * fa * (1.0 - sa)) / outA, 0.0, 255.0));
                }
                p[3] = static_cast<uint8_t>(std::clamp(outA * 255.0, 0.0, 255.0));
            }
        }
    }
}

} // namespace FreeEffect
