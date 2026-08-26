#include "../effect_registry.h"
#include "color_wheels_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<ColorWheelsEffect> s_reg("Color Wheels", "Color Correction");

ColorWheelsEffect::ColorWheelsEffect() {
    addParameter(EffectParameter::makeColor("liftColor", "Lift", {0.0, 0.0, 0.0, 1.0}));
    addParameter(EffectParameter::makeColor("gammaColor", "Gamma", {0.5, 0.5, 0.5, 1.0}));
    addParameter(EffectParameter::makeColor("gainColor", "Gain", {1.0, 1.0, 1.0, 1.0}));
    addParameter(EffectParameter::makeFloat("liftMaster", "Lift Master", -100.0, 100.0, 0.0));
    addParameter(EffectParameter::makeFloat("gammaMaster", "Gamma Master", -100.0, 100.0, 0.0));
    addParameter(EffectParameter::makeFloat("gainMaster", "Gain Master", -100.0, 100.0, 0.0));
}

std::unique_ptr<Effect> ColorWheelsEffect::clone() const {
    auto e = std::make_unique<ColorWheelsEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void ColorWheelsEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    Color lift = getColorParam("liftColor");
    Color gamma = getColorParam("gammaColor");
    Color gain = getColorParam("gainColor");
    float liftM = getFloatParam("liftMaster") / 100.0f;
    float gammaM = 1.0f + getFloatParam("gammaMaster") / 100.0f;
    float gainM = 1.0f + getFloatParam("gainMaster") / 100.0f;

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            float r = p[0] / 255.0f, g = p[1] / 255.0f, b = p[2] / 255.0f;
            float luma = 0.299f * r + 0.587f * g + 0.114f * b;

            float shadow = std::max(0.0f, 1.0f - luma * 2.0f);
            float mid = std::max(0.0f, 1.0f - std::abs(luma - 0.5f) * 2.0f);
            float highlight = std::max(0.0f, luma * 2.0f - 1.0f);

            r += lift.r * shadow * 0.5f * (1.0f + liftM);
            g += lift.g * shadow * 0.5f * (1.0f + liftM);
            b += lift.b * shadow * 0.5f * (1.0f + liftM);

            r = std::pow(std::max(r, 0.001f), 1.0f / std::max(gammaM, 0.01f)) * gamma.r;
            g = std::pow(std::max(g, 0.001f), 1.0f / std::max(gammaM, 0.01f)) * gamma.g;
            b = std::pow(std::max(b, 0.001f), 1.0f / std::max(gammaM, 0.01f)) * gamma.b;

            r *= gain.r * gainM;
            g *= gain.g * gainM;
            b *= gain.b * gainM;

            p[0] = static_cast<uint8_t>(std::clamp(r * 255.0f, 0.0f, 255.0f));
            p[1] = static_cast<uint8_t>(std::clamp(g * 255.0f, 0.0f, 255.0f));
            p[2] = static_cast<uint8_t>(std::clamp(b * 255.0f, 0.0f, 255.0f));
        }
    }
}

} // namespace FreeEffect
