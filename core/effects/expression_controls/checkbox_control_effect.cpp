#include "../effect_registry.h"
#include "checkbox_control_effect.h"

namespace FreeEffect {

static EffectRegistrar<CheckboxControlEffect> s_reg("Checkbox Control", "Expression Controls");

CheckboxControlEffect::CheckboxControlEffect() {
    addParameter(EffectParameter::makeBool("value", "Checkbox", false));
}

std::unique_ptr<Effect> CheckboxControlEffect::clone() const {
    auto e = std::make_unique<CheckboxControlEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void CheckboxControlEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    (void)buffer;
}

} // namespace FreeEffect
