#include "../effect_registry.h"
#include "saturation_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<SaturationEffect> s_reg("Saturation", "Color Correction");

SaturationEffect::SaturationEffect() {
    addParameter(EffectParameter::makeFloat("saturation", "Saturation", -100.0, 100.0, 0.0));
}

std::unique_ptr<Effect> SaturationEffect::clone() const {
    auto e = std::make_unique<SaturationEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void SaturationEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    float sat = 1.0f + getFloatParam("saturation") / 100.0f;

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            float luma = 0.299f * p[0] + 0.587f * p[1] + 0.114f * p[2];
            p[0] = static_cast<uint8_t>(std::clamp(luma + (p[0] - luma) * sat, 0.0f, 255.0f));
            p[1] = static_cast<uint8_t>(std::clamp(luma + (p[1] - luma) * sat, 0.0f, 255.0f));
            p[2] = static_cast<uint8_t>(std::clamp(luma + (p[2] - luma) * sat, 0.0f, 255.0f));
        }
    }
}

} // namespace FreeEffect
