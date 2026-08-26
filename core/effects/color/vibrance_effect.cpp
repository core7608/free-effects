#include "../effect_registry.h"
#include "vibrance_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<VibranceEffect> s_reg("Vibrance", "Color Correction");

VibranceEffect::VibranceEffect() {
    addParameter(EffectParameter::makeFloat("vibrance", "Vibrance", -100.0, 100.0, 0.0));
    addParameter(EffectParameter::makeFloat("saturation", "Saturation", -100.0, 100.0, 0.0));
}

std::vector<ParameterGroup> VibranceEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeFloat("vibrance", "Vibrance", -100.0, 100.0, false),
        EffectParameter::makeFloat("saturation", "Saturation", -100.0, 100.0, false)
    }}};
}

std::unique_ptr<Effect> VibranceEffect::clone() const {
    auto e = std::make_unique<VibranceEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void VibranceEffect::render(PixelBuffer& buffer, double time) {
    float vib = getFloatParam("vibrance") / 100.0f;
    float satAdj = 1.0f + getFloatParam("saturation") / 100.0f;

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            float r = p[0] / 255.0f, g = p[1] / 255.0f, b = p[2] / 255.0f;
            float maxC = std::max({r, g, b});
            float minC = std::min({r, g, b});
            float sat = (maxC > 0.001f) ? (maxC - minC) / maxC : 0.0f;

            float adj = satAdj + vib * (1.0f - sat);
            float avg = (r + g + b) / 3.0f;
            r = avg + (r - avg) * adj;
            g = avg + (g - avg) * adj;
            b = avg + (b - avg) * adj;
            p[0] = static_cast<uint8_t>(std::clamp(static_cast<double>(r * 255.0f), 0.0, 255.0));
            p[1] = static_cast<uint8_t>(std::clamp(static_cast<double>(g * 255.0f), 0.0, 255.0));
            p[2] = static_cast<uint8_t>(std::clamp(static_cast<double>(b * 255.0f), 0.0, 255.0));
        }
    }
}

} // namespace FreeEffect
