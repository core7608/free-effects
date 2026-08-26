#include "../effect_registry.h"
#include "slider_control_effect.h"

namespace FreeEffect {

static EffectRegistrar<SliderControlEffect> s_reg("Slider Control", "Expression Controls");

SliderControlEffect::SliderControlEffect() {
    addParameter(EffectParameter::makeFloat("value", "Slider", 0.0, 1000.0, 50.0));
}

std::unique_ptr<Effect> SliderControlEffect::clone() const {
    auto e = std::make_unique<SliderControlEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void SliderControlEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    (void)buffer;
}

} // namespace FreeEffect
