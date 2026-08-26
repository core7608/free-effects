#include "../../math/math_constants.h"
#include "../effect_registry.h"
#include "starglow_effect.h"
#include <cmath>
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<StarglowEffect> s_reg("Starglow", "Plugin Effect", "Trapcode");

StarglowEffect::StarglowEffect() {
    addParameter(EffectParameter::makeFloat("threshold", "Threshold", 0.0, 1.0, 0.5));
    addParameter(EffectParameter::makeFloat("glow_size", "Glow Size", 1.0, 50.0, 15.0));
    addParameter(EffectParameter::makeInt("streaks", "Streaks", 2, 8, 4));
    addParameter(EffectParameter::makeColor("color", "Color", Color{0.8, 0.7, 1.0, 1.0}));
    addParameter(EffectParameter::makeFloat("intensity", "Intensity", 0.0, 3.0, 1.0));
    addParameter(EffectParameter::makeAngle("direction", "Direction", 0.0));
}

std::vector<ParameterGroup> StarglowEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeFloat("threshold", "Threshold", 0.0, 1.0, 0.5),
        EffectParameter::makeFloat("glow_size", "Glow Size", 1.0, 50.0, 15.0),
        EffectParameter::makeInt("streaks", "Streaks", 2, 8, 4),
        EffectParameter::makeColor("color", "Color", Color{0.8, 0.7, 1.0, 1.0}),
        EffectParameter::makeFloat("intensity", "Intensity", 0.0, 3.0, 1.0),
        EffectParameter::makeAngle("direction", "Direction", 0.0)
    }}};
}

std::unique_ptr<Effect> StarglowEffect::clone() const {
    auto e = std::make_unique<StarglowEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void StarglowEffect::render(PixelBuffer& buffer, double time) {
    if (buffer.width == 0 || buffer.height == 0) return;
    double thresh = getFloatParam("threshold");
    double glowSz = getFloatParam("glow_size");
    int streaks = getIntParam("streaks");
    Color sc = getColorParam("color");
    double intensity = getFloatParam("intensity");
    double baseDir = getAngleParam("direction") * M_PI / 180.0;
    double cr = sc.r * 255.0;
    double cg = sc.g * 255.0;
    double cb = sc.b * 255.0;
    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);
    tmp.data = buffer.data;
    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            const uint8_t* p = tmp.pixelAt(x, y);
            double luma = (p[0] * 0.299 + p[1] * 0.587 + p[2] * 0.114) / 255.0;
            if (luma < thresh) continue;
            double bright = (luma - thresh) / (1.0 - thresh + 0.001) * intensity;
            for (int s = 0; s < streaks; s++) {
                double streakAngle = baseDir + s * 6.28318 / streaks;
                double dx = std::cos(streakAngle);
                double dy = std::sin(streakAngle);
                int len = static_cast<int>(glowSz * bright);
                for (int d = 1; d <= len; d++) {
                    double fade = 1.0 - static_cast<double>(d) / len;
                    fade = fade * fade;
                    int px = std::clamp(static_cast<int>(x + dx * d), 0, buffer.width - 1);
                    int py = std::clamp(static_cast<int>(y + dy * d), 0, buffer.height - 1);
                    double fa = fade * bright * sc.a * 0.5;
                    uint8_t* dst = buffer.pixelAt(px, py);
                    double sa = dst[3] / 255.0;
                    double outA = sa + fa * (1.0 - sa);
                    if (outA > 0.001) {
                        dst[0] = static_cast<uint8_t>((dst[0] * sa + cr * fa * (1.0 - sa)) / outA);
                        dst[1] = static_cast<uint8_t>((dst[1] * sa + cg * fa * (1.0 - sa)) / outA);
                        dst[2] = static_cast<uint8_t>((dst[2] * sa + cb * fa * (1.0 - sa)) / outA);
                    }
                    dst[3] = static_cast<uint8_t>(std::clamp(outA * 255.0, 0.0, 255.0));
                }
            }
        }
    }
}

} // namespace FreeEffect
