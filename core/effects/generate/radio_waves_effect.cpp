#include "../effect_registry.h"
#include "radio_waves_effect.h"
#include <cmath>
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<RadioWavesEffect> s_reg("Radio Waves", "Generate");

RadioWavesEffect::RadioWavesEffect() {
    addParameter(EffectParameter::makeVec2("center", "Center", Vec2{0.5, 0.5}));
    addParameter(EffectParameter::makeFloat("speed", "Expansion Speed", 0.1, 10.0, 2.0));
    addParameter(EffectParameter::makeFloat("fade", "Fade Out", 0.0, 5.0, 2.0));
    addParameter(EffectParameter::makeColor("color", "Color", Color{0.0, 0.5, 1.0, 0.8}));
    addParameter(EffectParameter::makeFloat("thickness", "Thickness", 1.0, 10.0, 2.0));
    addParameter(EffectParameter::makeInt("wave_count", "Wave Count", 1, 30, 10));
}

std::vector<ParameterGroup> RadioWavesEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeVec2("center", "Center", Vec2{0.5, 0.5}),
        EffectParameter::makeFloat("speed", "Expansion Speed", 0.1, 10.0, 2.0),
        EffectParameter::makeFloat("fade", "Fade Out", 0.0, 5.0, 2.0),
        EffectParameter::makeColor("color", "Color", Color{0.0, 0.5, 1.0, 0.8}),
        EffectParameter::makeFloat("thickness", "Thickness", 1.0, 10.0, 2.0),
        EffectParameter::makeInt("wave_count", "Wave Count", 1, 30, 10)
    }}};
}

std::unique_ptr<Effect> RadioWavesEffect::clone() const {
    auto e = std::make_unique<RadioWavesEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void RadioWavesEffect::render(PixelBuffer& buffer, double time) {
    if (buffer.width == 0 || buffer.height == 0) return;
    Vec2 ctr = getVec2Param("center");
    double spd = getFloatParam("speed");
    double fadeTime = getFloatParam("fade");
    Color wc = getColorParam("color");
    double thick = getFloatParam("thickness");
    int waveCount = getIntParam("wave_count");
    double cx = ctr.x * buffer.width;
    double cy = ctr.y * buffer.height;
    double maxR = std::sqrt(cx * cx + cy * cy) * 1.5;
    double interval = maxR / (spd * waveCount + 1.0);
    for (int w = 0; w < waveCount; w++) {
        double waveTime = std::fmod(time - w * interval, maxR / spd + interval);
        if (waveTime < 0) waveTime += maxR / spd + interval;
        double radius = waveTime * spd;
        if (radius > maxR || radius < 1.0) continue;
        double age = waveTime;
        double alpha = fadeTime > 0 ? std::max(0.0, 1.0 - age / (maxR / spd)) : 1.0;
        alpha *= wc.a;
        if (alpha < 0.01) continue;
        int iRadius = static_cast<int>(std::ceil(radius));
        int thickRad = static_cast<int>(std::ceil(thick / 2.0));
        for (int dy = -iRadius - thickRad; dy <= iRadius + thickRad; dy++) {
            for (int dx = -iRadius - thickRad; dx <= iRadius + thickRad; dx++) {
                int px = static_cast<int>(cx + dx);
                int py = static_cast<int>(cy + dy);
                if (px < 0 || px >= buffer.width || py < 0 || py >= buffer.height) continue;
                double dist = std::sqrt(dx * dx + dy * dy);
                double edgeDist = std::abs(dist - radius);
                if (edgeDist > thickRad) continue;
                double fade = 1.0 - edgeDist / (thickRad + 0.5);
                double fa = alpha * fade;
                uint8_t* p = buffer.pixelAt(px, py);
                double sa = p[3] / 255.0;
                double outA = sa + fa * (1.0 - sa);
                if (outA > 0.001) {
                    p[0] = static_cast<uint8_t>((p[0] * sa + wc.r * 255.0 * fa * (1.0 - sa)) / outA);
                    p[1] = static_cast<uint8_t>((p[1] * sa + wc.g * 255.0 * fa * (1.0 - sa)) / outA);
                    p[2] = static_cast<uint8_t>((p[2] * sa + wc.b * 255.0 * fa * (1.0 - sa)) / outA);
                }
                p[3] = static_cast<uint8_t>(std::clamp(outA * 255.0, 0.0, 255.0));
            }
        }
    }
}

} // namespace FreeEffect
