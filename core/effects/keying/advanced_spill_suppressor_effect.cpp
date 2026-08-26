#include "../effect_registry.h"
#include "advanced_spill_suppressor_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<AdvancedSpillSuppressorEffect> s_reg("Advanced Spill Suppressor", "Keying");

AdvancedSpillSuppressorEffect::AdvancedSpillSuppressorEffect() {
    addParameter(EffectParameter::makeDropdown("spillColor", "Key Color", {"Red", "Green", "Blue"}, 1));
    addParameter(EffectParameter::makeFloat("suppression", "Suppression", 0.0, 100.0, 100.0));
    addParameter(EffectParameter::makeFloat("luminanceSoft", "Luminance Soft", 0.0, 100.0, 50.0));
}

std::unique_ptr<Effect> AdvancedSpillSuppressorEffect::clone() const {
    auto e = std::make_unique<AdvancedSpillSuppressorEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void AdvancedSpillSuppressorEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    int spillCh = getDropdownParam("spillColor");
    float supp = getFloatParam("suppression") / 100.0f;

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            float r = p[0], g = p[1], b = p[2];
            float luma = 0.299f * r + 0.587f * g + 0.114f * b;
            float spillVal = 0;
            if (spillCh == 0) spillVal = r - luma;
            else if (spillCh == 1) spillVal = g - luma;
            else spillVal = b - luma;
            if (spillVal > 0) {
                float reduce = spillVal * supp;
                if (spillCh == 0) p[0] = static_cast<uint8_t>(std::clamp(r - reduce, 0.0f, 255.0f));
                else if (spillCh == 1) p[1] = static_cast<uint8_t>(std::clamp(g - reduce, 0.0f, 255.0f));
                else p[2] = static_cast<uint8_t>(std::clamp(b - reduce, 0.0f, 255.0f));
            }
        }
    }
}

} // namespace FreeEffect
