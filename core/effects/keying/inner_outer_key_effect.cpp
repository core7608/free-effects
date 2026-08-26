#include "../effect_registry.h"
#include "inner_outer_key_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<InnerOuterKeyEffect> s_reg("Inner/Outer Key", "Keying");

InnerOuterKeyEffect::InnerOuterKeyEffect() {
    addParameter(EffectParameter::makeColor("innerKeyColor", "Inner Key Color", {0.0, 255.0, 0.0, 1.0}));
    addParameter(EffectParameter::makeColor("outerKeyColor", "Outer Key Color", {0.0, 200.0, 0.0, 1.0}));
    addParameter(EffectParameter::makeFloat("innerTolerance", "Inner Tolerance", 0.0, 255.0, 50.0));
    addParameter(EffectParameter::makeFloat("outerTolerance", "Outer Tolerance", 0.0, 255.0, 100.0));
}

std::unique_ptr<Effect> InnerOuterKeyEffect::clone() const {
    auto e = std::make_unique<InnerOuterKeyEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void InnerOuterKeyEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    Color inner = getColorParam("innerKeyColor"), outer = getColorParam("outerKeyColor");
    float innerT = getFloatParam("innerTolerance");
    float outerT = getFloatParam("outerTolerance");

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            float dInner = std::sqrt(std::pow(p[0]-inner.r,2) + std::pow(p[1]-inner.g,2) + std::pow(p[2]-inner.b,2));
            float dOuter = std::sqrt(std::pow(p[0]-outer.r,2) + std::pow(p[1]-outer.g,2) + std::pow(p[2]-outer.b,2));
            float innerAlpha = std::clamp(1.0f - dInner / innerT, 0.0f, 1.0f);
            float outerAlpha = std::clamp(dOuter / outerT, 0.0f, 1.0f);
            float finalAlpha = std::max(innerAlpha, outerAlpha);
            p[3] = static_cast<uint8_t>(finalAlpha * 255.0f);
        }
    }
}

} // namespace FreeEffect
