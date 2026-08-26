#include "../effect_registry.h"
#include "fill_effect.h"
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<FillEffect> s_reg("Fill", "Generate");

FillEffect::FillEffect() {
    addParameter(EffectParameter::makeColor("color", "Color", {255.0, 0.0, 0.0, 1.0}));
}

std::vector<ParameterGroup> FillEffect::getParameterGroups() const {
    return {{getName(), {EffectParameter::makeColor("color", "Color", {255.0, 0.0, 0.0, 1.0})}}};
}

std::unique_ptr<Effect> FillEffect::clone() const {
    auto e = std::make_unique<FillEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void FillEffect::render(PixelBuffer& buffer, double time) {
    Color c = getColorParam("color");
    uint8_t r = static_cast<uint8_t>(std::clamp(c.r, 0.0, 255.0));
    uint8_t g = static_cast<uint8_t>(std::clamp(c.g, 0.0, 255.0));
    uint8_t b = static_cast<uint8_t>(std::clamp(c.b, 0.0, 255.0));
    uint8_t a = static_cast<uint8_t>(std::clamp(c.a * 255.0, 0.0, 255.0));

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            float srcA = p[3] / 255.0f;
            float fillA = a / 255.0f;
            float outA = fillA + srcA * (1.0f - fillA);
            if (outA > 0) {
                p[0] = static_cast<uint8_t>((r * fillA + p[0] * srcA * (1.0f - fillA)) / outA);
                p[1] = static_cast<uint8_t>((g * fillA + p[1] * srcA * (1.0f - fillA)) / outA);
                p[2] = static_cast<uint8_t>((b * fillA + p[2] * srcA * (1.0f - fillA)) / outA);
            }
            p[3] = static_cast<uint8_t>(outA * 255.0f);
        }
    }
}

} // namespace FreeEffect
