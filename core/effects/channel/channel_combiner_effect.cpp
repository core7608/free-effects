#include "../effect_registry.h"
#include "channel_combiner_effect.h"
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<ChannelCombinerEffect> s_reg("Channel Combiner", "Channel");

ChannelCombinerEffect::ChannelCombinerEffect() {
    addParameter(EffectParameter::makeDropdown("fromRed", "Red Source", {"Red", "Green", "Blue", "Alpha", "Luminance"}, 0));
    addParameter(EffectParameter::makeDropdown("fromGreen", "Green Source", {"Red", "Green", "Blue", "Alpha", "Luminance"}, 1));
    addParameter(EffectParameter::makeDropdown("fromBlue", "Blue Source", {"Red", "Green", "Blue", "Alpha", "Luminance"}, 2));
    addParameter(EffectParameter::makeDropdown("fromAlpha", "Alpha Source", {"Red", "Green", "Blue", "Alpha", "Luminance"}, 3));
}

std::unique_ptr<Effect> ChannelCombinerEffect::clone() const {
    auto e = std::make_unique<ChannelCombinerEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void ChannelCombinerEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    int fromR = getDropdownParam("fromRed");
    int fromG = getDropdownParam("fromGreen");
    int fromB = getDropdownParam("fromBlue");
    int fromA = getDropdownParam("fromAlpha");

    auto getChannel = [](const uint8_t* p, int ch) -> float {
        if (ch < 3) return p[ch] / 255.0f;
        if (ch == 3) return p[3] / 255.0f;
        return (0.299f * p[0] + 0.587f * p[1] + 0.114f * p[2]) / 255.0f;
    };

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            uint8_t src[4] = {p[0], p[1], p[2], p[3]};
            p[0] = static_cast<uint8_t>(std::clamp(getChannel(src, fromR) * 255.0f, 0.0f, 255.0f));
            p[1] = static_cast<uint8_t>(std::clamp(getChannel(src, fromG) * 255.0f, 0.0f, 255.0f));
            p[2] = static_cast<uint8_t>(std::clamp(getChannel(src, fromB) * 255.0f, 0.0f, 255.0f));
            p[3] = static_cast<uint8_t>(std::clamp(getChannel(src, fromA) * 255.0f, 0.0f, 255.0f));
        }
    }
}

} // namespace FreeEffect
