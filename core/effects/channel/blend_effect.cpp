#include "../effect_registry.h"
#include "blend_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<BlendEffect> s_reg("Blend", "Channel");

BlendEffect::BlendEffect() {
    addParameter(EffectParameter::makeFloat("opacity", "Opacity", 0.0, 100.0, 50.0));
    addParameter(EffectParameter::makeDropdown("mode", "Blend Mode",
        {"Normal", "Add", "Multiply", "Screen", "Overlay", "Difference", "Subtract",
         "Darken", "Lighten", "Color Dodge", "Color Burn", "Soft Light", "Hard Light"}, 0));
    addParameter(EffectParameter::makeDropdown("source", "Source", {"Foreground on Background", "Foreground"}, 0));
}

std::unique_ptr<Effect> BlendEffect::clone() const {
    auto e = std::make_unique<BlendEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void BlendEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    float opacity = getFloatParam("opacity") / 100.0f;
    int mode = getDropdownParam("mode");

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            float luma = (0.299f * p[0] + 0.587f * p[1] + 0.114f * p[2]) / 255.0f;

            for (int c = 0; c < 3; c++) {
                float fg = p[c] / 255.0f;
                float bg = luma;
                float result = fg;

                switch (mode) {
                    case 0: result = fg; break;
                    case 1: result = fg + bg; break;
                    case 2: result = fg * bg; break;
                    case 3: result = 1.0f - (1.0f - fg) * (1.0f - bg); break;
                    case 4: result = bg < 0.5f ? 2.0f * fg * bg : 1.0f - 2.0f * (1.0f - fg) * (1.0f - bg); break;
                    case 5: result = std::abs(fg - bg); break;
                    case 6: result = fg - bg; break;
                    case 7: result = std::min(fg, bg); break;
                    case 8: result = std::max(fg, bg); break;
                    case 9: result = bg / std::max(1.0f - fg, 0.001f); break;
                    case 10: result = 1.0f - (1.0f - bg) / std::max(fg, 0.001f); break;
                    case 11: result = bg < 0.5f ? 2.0f * fg * bg + fg * fg * (1.0f - 2.0f * bg) : std::sqrt(bg) * (2.0f * fg - 1.0f) + 2.0f * bg * (1.0f - fg); break;
                    case 12: result = fg < 0.5f ? 2.0f * fg * bg : 1.0f - 2.0f * (1.0f - fg) * (1.0f - bg); break;
                }

                result = std::clamp(result, 0.0f, 1.0f);
                p[c] = static_cast<uint8_t>(fg * 255.0f * (1.0f - opacity) + result * 255.0f * opacity);
            }
        }
    }
}

} // namespace FreeEffect
