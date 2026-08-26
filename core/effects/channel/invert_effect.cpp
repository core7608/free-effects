#include "../effect_registry.h"
#include "invert_effect.h"
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<InvertEffect> s_reg("Invert", "Channel");

InvertEffect::InvertEffect() {
    addParameter(EffectParameter::makeDropdown("channel", "Channel", {"Red", "Green", "Blue", "RGB", "Alpha"}, 3));
}

std::unique_ptr<Effect> InvertEffect::clone() const {
    auto e = std::make_unique<InvertEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void InvertEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    int channel = getDropdownParam("channel");

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            if (channel == 0) p[0] = 255 - p[0];
            else if (channel == 1) p[1] = 255 - p[1];
            else if (channel == 2) p[2] = 255 - p[2];
            else if (channel == 3) { p[0] = 255 - p[0]; p[1] = 255 - p[1]; p[2] = 255 - p[2]; }
            else if (channel == 4) p[3] = 255 - p[3];
        }
    }
}

} // namespace FreeEffect
