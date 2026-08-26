#include "../effect_registry.h"
#include "tint_effect.h"
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<TintEffect> s_reg("Tint", "Color Correction");

TintEffect::TintEffect() {
    addParameter(EffectParameter::makeColor("mapBlackTo", "Map Black To", {0.0, 0.0, 0.0, 1.0}));
    addParameter(EffectParameter::makeColor("mapWhiteTo", "Map White To", {255.0, 255.0, 255.0, 1.0}));
    addParameter(EffectParameter::makeFloat("amount", "Amount To Tint", 0.0, 100.0, 100.0));
}

std::vector<ParameterGroup> TintEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeColor("mapBlackTo", "Map Black To", {0.0, 0.0, 0.0, 1.0}),
        EffectParameter::makeColor("mapWhiteTo", "Map White To", {255.0, 255.0, 255.0, 1.0}),
        EffectParameter::makeFloat("amount", "Amount To Tint", 0.0, 100.0, false)
    }}};
}

std::unique_ptr<Effect> TintEffect::clone() const {
    auto e = std::make_unique<TintEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void TintEffect::render(PixelBuffer& buffer, double time) {
    Color black = getColorParam("mapBlackTo");
    Color white = getColorParam("mapWhiteTo");
    float amount = getFloatParam("amount") / 100.0f;

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            float lum = (p[0] * 0.299f + p[1] * 0.587f + p[2] * 0.114f) / 255.0f;
            float r = black.r + (white.r - black.r) * lum;
            float g = black.g + (white.g - black.g) * lum;
            float b = black.b + (white.b - black.b) * lum;
            p[0] = static_cast<uint8_t>(std::clamp(static_cast<double>(p[0] + (r - p[0]) * amount), 0.0, 255.0));
            p[1] = static_cast<uint8_t>(std::clamp(static_cast<double>(p[1] + (g - p[1]) * amount), 0.0, 255.0));
            p[2] = static_cast<uint8_t>(std::clamp(static_cast<double>(p[2] + (b - p[2]) * amount), 0.0, 255.0));
        }
    }
}

} // namespace FreeEffect
