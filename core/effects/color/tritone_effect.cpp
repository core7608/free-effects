#include "../effect_registry.h"
#include "tritone_effect.h"
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<TritoneEffect> s_reg("Tritone", "Color Correction");

TritoneEffect::TritoneEffect() {
    addParameter(EffectParameter::makeColor("highlights", "Highlights", {255.0, 255.0, 204.0, 1.0}));
    addParameter(EffectParameter::makeColor("midtones", "Midtones", {178.0, 102.0, 51.0, 1.0}));
    addParameter(EffectParameter::makeColor("shadows", "Shadows", {51.0, 0.0, 51.0, 1.0}));
}

std::vector<ParameterGroup> TritoneEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeColor("highlights", "Highlights", {255.0, 255.0, 204.0, 1.0}),
        EffectParameter::makeColor("midtones", "Midtones", {178.0, 102.0, 51.0, 1.0}),
        EffectParameter::makeColor("shadows", "Shadows", {51.0, 0.0, 51.0, 1.0})
    }}};
}

std::unique_ptr<Effect> TritoneEffect::clone() const {
    auto e = std::make_unique<TritoneEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void TritoneEffect::render(PixelBuffer& buffer, double time) {
    Color hi = getColorParam("highlights");
    Color mid = getColorParam("midtones");
    Color sh = getColorParam("shadows");

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            float lum = (p[0] * 0.299f + p[1] * 0.587f + p[2] * 0.114f) / 255.0f;
            float r, g, b;
            if (lum < 0.5f) {
                float t = lum * 2.0f;
                r = sh.r * (1.0f - t) + mid.r * t;
                g = sh.g * (1.0f - t) + mid.g * t;
                b = sh.b * (1.0f - t) + mid.b * t;
            } else {
                float t = (lum - 0.5f) * 2.0f;
                r = mid.r * (1.0f - t) + hi.r * t;
                g = mid.g * (1.0f - t) + hi.g * t;
                b = mid.b * (1.0f - t) + hi.b * t;
            }
            p[0] = static_cast<uint8_t>(std::clamp(static_cast<double>(r), 0.0, 255.0));
            p[1] = static_cast<uint8_t>(std::clamp(static_cast<double>(g), 0.0, 255.0));
            p[2] = static_cast<uint8_t>(std::clamp(static_cast<double>(b), 0.0, 255.0));
        }
    }
}

} // namespace FreeEffect
