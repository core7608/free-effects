#include "../effect_registry.h"
#include "point_3d_control_effect.h"

namespace FreeEffect {

static EffectRegistrar<Point3DControlEffect> s_reg("3D Point Control", "Expression Controls");

Point3DControlEffect::Point3DControlEffect() {
    addParameter(EffectParameter::makeFloat("x", "X", -10000.0, 10000.0, 0.0));
    addParameter(EffectParameter::makeFloat("y", "Y", -10000.0, 10000.0, 0.0));
    addParameter(EffectParameter::makeFloat("z", "Z", -10000.0, 10000.0, 0.0));
}

std::unique_ptr<Effect> Point3DControlEffect::clone() const {
    auto e = std::make_unique<Point3DControlEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void Point3DControlEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    (void)buffer;
}

} // namespace FreeEffect
