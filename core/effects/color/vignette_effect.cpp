#include "../effect_registry.h"
#include "vignette_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<VignetteEffect> s_reg("Vignette", "Color Correction");

VignetteEffect::VignetteEffect() {
    addParameter(EffectParameter::makeVec2("center", "Center", {0.5, 0.5}));
    addParameter(EffectParameter::makeFloat("amount", "Amount", -100.0, 100.0, -50.0));
    addParameter(EffectParameter::makeFloat("roundness", "Roundness", -100.0, 100.0, 0.0));
    addParameter(EffectParameter::makeFloat("softness", "Softness", 0.0, 100.0, 50.0));
    addParameter(EffectParameter::makeColor("color", "Color", {0.0, 0.0, 0.0, 1.0}));
}

std::unique_ptr<Effect> VignetteEffect::clone() const {
    auto e = std::make_unique<VignetteEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void VignetteEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    Vec2 center = getVec2Param("center");
    float amount = getFloatParam("amount") / 100.0f;
    float roundness = 1.0f + getFloatParam("roundness") / 100.0f;
    float softness = getFloatParam("softness") / 100.0f;
    Color vColor = getColorParam("color");

    float cx = center.x * buffer.width;
    float cy = center.y * buffer.height;
    float maxDist = std::sqrt(cx * cx + cy * cy);
    if (maxDist < 1.0f) maxDist = 1.0f;

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            float dx = (x - cx) / (buffer.width * 0.5f);
            float dy = (y - cy) / (buffer.height * 0.5f * roundness);
            float dist = std::sqrt(dx * dx + dy * dy);
            float vignette = std::clamp((dist - (1.0f - softness)) / (softness + 0.001f), 0.0f, 1.0f);

            if (amount < 0) vignette *= -amount;
            else vignette = 1.0f - (1.0f - vignette) * amount;

            uint8_t* p = buffer.pixelAt(x, y);
            p[0] = static_cast<uint8_t>(std::clamp(static_cast<double>(p[0] * (1.0f - vignette) + vColor.r * vignette), 0.0, 255.0));
            p[1] = static_cast<uint8_t>(std::clamp(static_cast<double>(p[1] * (1.0f - vignette) + vColor.g * vignette), 0.0, 255.0));
            p[2] = static_cast<uint8_t>(std::clamp(static_cast<double>(p[2] * (1.0f - vignette) + vColor.b * vignette), 0.0, 255.0));
        }
    }
}

} // namespace FreeEffect
