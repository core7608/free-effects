#include "../effect_registry.h"
#include "change_to_color_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<ChangeToColorEffect> s_reg("Change To Color", "Color Correction");

ChangeToColorEffect::ChangeToColorEffect() {
    addParameter(EffectParameter::makeColor("fromColor", "From Color", {255.0, 0.0, 0.0, 1.0}));
    addParameter(EffectParameter::makeColor("toColor", "To Color", {0.0, 0.0, 255.0, 1.0}));
    addParameter(EffectParameter::makeFloat("tolerance", "Tolerance", 0.0, 100.0, 30.0));
    addParameter(EffectParameter::makeBool("softness", "Softness", true));
    addParameter(EffectParameter::makeDropdown("changeBy", "Change By", {"Hue", "Hue & Lightness", "Hue, Lightness, Saturation"}, 2));
}

std::unique_ptr<Effect> ChangeToColorEffect::clone() const {
    auto e = std::make_unique<ChangeToColorEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void ChangeToColorEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    Color from = getColorParam("fromColor");
    Color to = getColorParam("toColor");
    float tolerance = getFloatParam("tolerance") / 100.0f;
    int changeBy = getDropdownParam("changeBy");

    float fr = from.r, fg = from.g, fb = from.b;
    float tr = to.r / 255.0f, tg = to.g / 255.0f, tb = to.b / 255.0f;

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            float dr = p[0] - fr, dg = p[1] - fg, db = p[2] - fb;
            float dist = std::sqrt(dr * dr + dg * dg + db * db) / 255.0f;
            float match = std::clamp(1.0f - dist / (tolerance + 0.001f), 0.0f, 1.0f);

            if (match > 0) {
                float r = p[0] / 255.0f, g = p[1] / 255.0f, b = p[2] / 255.0f;
                float cmax = std::max({r, g, b}), cmin = std::min({r, g, b});
                float luma = 0.299f * r + 0.587f * g + 0.114f * b;
                float tLuma = 0.299f * tr + 0.587f * tg + 0.114f * tb;

                float nr, ng, nb;
                if (changeBy == 0) {
                    float origSat = (cmax > 0.001f) ? (cmax - cmin) / cmax : 0;
                    nr = tr * origSat * cmax + r * (1.0f - origSat);
                    ng = tg * origSat * cmax + g * (1.0f - origSat);
                    nb = tb * origSat * cmax + b * (1.0f - origSat);
                } else if (changeBy == 1) {
                    nr = tr * luma / std::max(tLuma, 0.001f);
                    ng = tg * luma / std::max(tLuma, 0.001f);
                    nb = tb * luma / std::max(tLuma, 0.001f);
                } else {
                    nr = tr; ng = tg; nb = tb;
                }

                r = r + (nr - r) * match;
                g = g + (ng - g) * match;
                b = b + (nb - b) * match;

                p[0] = static_cast<uint8_t>(std::clamp(r * 255.0f, 0.0f, 255.0f));
                p[1] = static_cast<uint8_t>(std::clamp(g * 255.0f, 0.0f, 255.0f));
                p[2] = static_cast<uint8_t>(std::clamp(b * 255.0f, 0.0f, 255.0f));
            }
        }
    }
}

} // namespace FreeEffect
