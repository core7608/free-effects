#include "../effect_registry.h"
#include "difference_matte_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<DifferenceMatteEffect> s_reg("Difference Matte", "Keying");

DifferenceMatteEffect::DifferenceMatteEffect() {
    addParameter(EffectParameter::makeString("differenceLayer", "Difference Layer", ""));
    addParameter(EffectParameter::makeFloat("tolerance", "Tolerance", 0.0, 255.0, 25.0));
    addParameter(EffectParameter::makeFloat("edgeThin", "Edge Thin", -100.0, 100.0, 0.0));
}

std::vector<ParameterGroup> DifferenceMatteEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeString("differenceLayer", "Difference Layer", ""),
        EffectParameter::makeFloat("tolerance", "Tolerance", 0.0, 255.0, false),
        EffectParameter::makeFloat("edgeThin", "Edge Thin", -100.0, 100.0, false)
    }}};
}

std::unique_ptr<Effect> DifferenceMatteEffect::clone() const {
    auto e = std::make_unique<DifferenceMatteEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void DifferenceMatteEffect::render(PixelBuffer& buffer, double time) {
    float tolerance = getFloatParam("tolerance");

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            float lum = p[0] * 0.299f + p[1] * 0.587f + p[2] * 0.114f;
            float diff = std::abs(lum - 128.0f);
            float alpha = std::clamp(diff / std::max(tolerance, 0.001f), 0.0f, 1.0f);
            p[3] = static_cast<uint8_t>(alpha * 255.0f);
        }
    }
}

} // namespace FreeEffect
