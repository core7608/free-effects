#include "../effect_registry.h"
#include "layer_control_effect.h"

namespace FreeEffect {

static EffectRegistrar<LayerControlEffect> s_reg("Layer Control", "Expression Controls");

LayerControlEffect::LayerControlEffect() {
    addParameter(EffectParameter::makeDropdown("sourceLayer", "Source Layer",
        {"Layer 1", "Layer 2", "Layer 3", "Layer 4", "Layer 5"}, 0));
}

std::unique_ptr<Effect> LayerControlEffect::clone() const {
    auto e = std::make_unique<LayerControlEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void LayerControlEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    (void)buffer;
}

} // namespace FreeEffect
