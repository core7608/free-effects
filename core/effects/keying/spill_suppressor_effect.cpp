#include "../effect_registry.h"
#include "spill_suppressor_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<SpillSuppressorEffect> s_reg("Spill Suppressor", "Keying");

SpillSuppressorEffect::SpillSuppressorEffect() {
    addParameter(EffectParameter::makeFloat("suppression", "Suppression", 0.0, 100.0, 100.0));
    addParameter(EffectParameter::makeDropdown("spillChannel", "Spill", {"Red", "Green", "Blue"}, 1));
}

std::unique_ptr<Effect> SpillSuppressorEffect::clone() const {
    auto e = std::make_unique<SpillSuppressorEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void SpillSuppressorEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    float supp = getFloatParam("suppression") / 100.0f;
    int ch = getDropdownParam("spillChannel");

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            float luma = 0.299f * p[0] + 0.587f * p[1] + 0.114f * p[2];
            float spill = p[ch] - luma;
            if (spill > 0) {
                p[ch] = static_cast<uint8_t>(std::clamp(static_cast<double>(p[ch] - spill * supp), 0.0, 255.0));
            }
        }
    }
}

} // namespace FreeEffect
