#include "../effect_registry.h"
#include "channel_extract_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<ChannelExtractEffect> s_reg("Channel Extract", "3D Channel");

ChannelExtractEffect::ChannelExtractEffect() {
    addParameter(EffectParameter::makeDropdown("channel", "Source", {"Red", "Green", "Blue", "Alpha"}, 0));
}

std::unique_ptr<Effect> ChannelExtractEffect::clone() const {
    auto e = std::make_unique<ChannelExtractEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void ChannelExtractEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    int ch = getDropdownParam("channel");
    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            uint8_t val = p[ch];
            p[0] = val; p[1] = val; p[2] = val; p[3] = 255;
        }
    }
}

} // namespace FreeEffect
