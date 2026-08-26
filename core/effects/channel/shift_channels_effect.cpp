#include "../effect_registry.h"
#include "shift_channels_effect.h"
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<ShiftChannelsEffect> s_reg("Shift Channels", "Channel");

ShiftChannelsEffect::ShiftChannelsEffect() {
    addParameter(EffectParameter::makeDropdown("fromRedChannel", "Take Red Channel From",
        {"Red", "Green", "Blue", "Alpha"}, 0));
    addParameter(EffectParameter::makeDropdown("fromGreenChannel", "Take Green Channel From",
        {"Red", "Green", "Blue", "Alpha"}, 1));
    addParameter(EffectParameter::makeDropdown("fromBlueChannel", "Take Blue Channel From",
        {"Red", "Green", "Blue", "Alpha"}, 2));
    addParameter(EffectParameter::makeDropdown("fromAlphaChannel", "Take Alpha Channel From",
        {"Red", "Green", "Blue", "Alpha"}, 3));
}

std::unique_ptr<Effect> ShiftChannelsEffect::clone() const {
    auto e = std::make_unique<ShiftChannelsEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void ShiftChannelsEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    int fromR = getDropdownParam("fromRedChannel");
    int fromG = getDropdownParam("fromGreenChannel");
    int fromB = getDropdownParam("fromBlueChannel");
    int fromA = getDropdownParam("fromAlphaChannel");

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            uint8_t src[4] = {p[0], p[1], p[2], p[3]};
            p[0] = src[std::clamp(fromR, 0, 3)];
            p[1] = src[std::clamp(fromG, 0, 3)];
            p[2] = src[std::clamp(fromB, 0, 3)];
            p[3] = src[std::clamp(fromA, 0, 3)];
        }
    }
}

} // namespace FreeEffect
