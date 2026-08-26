#include "../effect_registry.h"
#include "remove_color_matting_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<RemoveColorMattingEffect> s_reg("Remove Color Matting", "Channel");

RemoveColorMattingEffect::RemoveColorMattingEffect() {
    addParameter(EffectParameter::makeColor("premultiplyColor", "Premultiply Color", {0.0, 0.0, 0.0, 1.0}));
    addParameter(EffectParameter::makeDropdown("colorType", "Color Type", {"Black", "White", "Custom"}, 0));
}

std::unique_ptr<Effect> RemoveColorMattingEffect::clone() const {
    auto e = std::make_unique<RemoveColorMattingEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void RemoveColorMattingEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    int colorType = getDropdownParam("colorType");
    Color premulColor = getColorParam("premultiplyColor");

    float bgR, bgG, bgB;
    if (colorType == 0) { bgR = 0; bgG = 0; bgB = 0; }
    else if (colorType == 1) { bgR = 255; bgG = 255; bgB = 255; }
    else { bgR = premulColor.r; bgG = premulColor.g; bgB = premulColor.b; }

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            float alpha = p[3] / 255.0f;
            if (alpha > 0.01f) {
                float r = (p[0] - bgR * alpha) / (1.0f - alpha + 0.001f);
                float g = (p[1] - bgG * alpha) / (1.0f - alpha + 0.001f);
                float b = (p[2] - bgB * alpha) / (1.0f - alpha + 0.001f);
                p[0] = static_cast<uint8_t>(std::clamp(r, 0.0f, 255.0f));
                p[1] = static_cast<uint8_t>(std::clamp(g, 0.0f, 255.0f));
                p[2] = static_cast<uint8_t>(std::clamp(b, 0.0f, 255.0f));
            }
        }
    }
}

} // namespace FreeEffect
