#include "../effect_registry.h"
#include "angle_control_effect.h"

namespace FreeEffect {

static EffectRegistrar<AngleControlEffect> s_reg("Angle Control", "Expression Controls");

AngleControlEffect::AngleControlEffect() {
    addParameter(EffectParameter::makeAngle("value", "Angle", 0.0));
}

std::unique_ptr<Effect> AngleControlEffect::clone() const {
    auto e = std::make_unique<AngleControlEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void AngleControlEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    (void)buffer;
}

} // namespace FreeEffect
