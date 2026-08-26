#include "../effect_registry.h"
#include "point_control_effect.h"

namespace FreeEffect {

static EffectRegistrar<PointControlEffect> s_reg("Point Control", "Expression Controls");

PointControlEffect::PointControlEffect() {
    addParameter(EffectParameter::makeFloat("x", "X", -10000.0, 10000.0, 0.0));
    addParameter(EffectParameter::makeFloat("y", "Y", -10000.0, 10000.0, 0.0));
}

std::unique_ptr<Effect> PointControlEffect::clone() const {
    auto e = std::make_unique<PointControlEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void PointControlEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    (void)buffer;
}

} // namespace FreeEffect
