#include "../effect_registry.h"
#include "color_control_effect.h"

namespace FreeEffect {

static EffectRegistrar<ColorControlEffect> s_reg("Color Control", "Expression Controls");

ColorControlEffect::ColorControlEffect() {
    addParameter(EffectParameter::makeColor("value", "Color", Color{1.0, 1.0, 1.0, 1.0}));
}

std::unique_ptr<Effect> ColorControlEffect::clone() const {
    auto e = std::make_unique<ColorControlEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void ColorControlEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    (void)buffer;
}

} // namespace FreeEffect
