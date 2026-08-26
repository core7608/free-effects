#include "../effect_registry.h"
#include "set_channels_effect.h"
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<SetChannelsEffect> s_reg("Set Channels", "Channel");

SetChannelsEffect::SetChannelsEffect() {
    addParameter(EffectParameter::makeInt("redFrom", "Red From Layer", 0, 99, 0));
    addParameter(EffectParameter::makeInt("greenFrom", "Green From Layer", 0, 99, 0));
    addParameter(EffectParameter::makeInt("blueFrom", "Blue From Layer", 0, 99, 0));
    addParameter(EffectParameter::makeInt("alphaFrom", "Alpha From Layer", 0, 99, 0));
}

std::unique_ptr<Effect> SetChannelsEffect::clone() const {
    auto e = std::make_unique<SetChannelsEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void SetChannelsEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    (void)buffer;
}

} // namespace FreeEffect
