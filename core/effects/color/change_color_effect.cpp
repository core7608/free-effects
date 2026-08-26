#include "../effect_registry.h"
#include "change_color_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<ChangeColorEffect> s_reg("Change Color", "Color Correction");

ChangeColorEffect::ChangeColorEffect() {
    addParameter(EffectParameter::makeDropdown("colorToChange", "Color To Change",
        {"Reds", "Yellows", "Greens", "Cyans", "Blues", "Magentas"}, 0));
    addParameter(EffectParameter::makeColor("newColor", "New Color", {255.0, 0.0, 0.0, 1.0}));
    addParameter(EffectParameter::makeFloat("hueTransform", "Hue Transform", -180.0, 180.0, 0.0));
    addParameter(EffectParameter::makeFloat("saturationTransform", "Saturation Transform", -100.0, 100.0, 0.0));
    addParameter(EffectParameter::makeFloat("lightnessTransform", "Lightness Transform", -100.0, 100.0, 0.0));
    addParameter(EffectParameter::makeFloat("tolerance", "Tolerance", 0.0, 100.0, 50.0));
    addParameter(EffectParameter::makeBool("softness", "Softness", true));
}

std::unique_ptr<Effect> ChangeColorEffect::clone() const {
    auto e = std::make_unique<ChangeColorEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void ChangeColorEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    int colorIdx = getDropdownParam("colorToChange");
    Color newC = getColorParam("newColor");
    float hueT = getFloatParam("hueTransform") / 360.0f;
    float satT = getFloatParam("saturationTransform") / 100.0f;
    float litT = getFloatParam("lightnessTransform") / 100.0f;
    float tolerance = getFloatParam("tolerance") / 100.0f;

    float hueRanges[][2] = {{0.0f, 0.08f}, {0.12f, 0.22f}, {0.25f, 0.45f}, {0.45f, 0.55f}, {0.55f, 0.75f}, {0.75f, 0.95f}};
    float targetHue = (hueRanges[colorIdx][0] + hueRanges[colorIdx][1]) / 2.0f;
    float hueRange = (hueRanges[colorIdx][1] - hueRanges[colorIdx][0]) / 2.0f;

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            float r = p[0] / 255.0f, g = p[1] / 255.0f, b = p[2] / 255.0f;

            float cmax = std::max({r, g, b}), cmin = std::min({r, g, b});
            float delta = cmax - cmin;
            float h = 0, s = 0, l = (cmax + cmin) / 2.0f;

            if (delta > 0.001f) {
                s = l > 0.5f ? delta / (2.0f - cmax - cmin) : delta / (cmax + cmin);
                if (cmax == r) h = (g - b) / delta + (g < b ? 6.0f : 0.0f);
                else if (cmax == g) h = (b - r) / delta + 2.0f;
                else h = (r - g) / delta + 4.0f;
                h /= 6.0f;
            }

            float dist = std::abs(h - targetHue);
            if (dist > 0.5f) dist = 1.0f - dist;
            float match = std::clamp(1.0f - dist / (hueRange * tolerance + 0.001f), 0.0f, 1.0f);

            if (match > 0) {
                h = std::fmod(h + hueT + 1.0f, 1.0f);
                s = std::clamp(s + satT, 0.0f, 1.0f);
                l = std::clamp(l + litT, 0.0f, 1.0f);

                float q = l < 0.5f ? l * (1.0f + s) : l + s - l * s;
                float p2 = 2.0f * l - q;
                auto h2r = [](float p2, float q2, float t) -> float {
                    if (t < 0) t += 1.0f; if (t > 1) t -= 1.0f;
                    if (t < 1.0f/6.0f) return p2 + (q2 - p2) * 6.0f * t;
                    if (t < 0.5f) return q2;
                    if (t < 2.0f/3.0f) return p2 + (q2 - p2) * (2.0f/3.0f - t) * 6.0f;
                    return p2;
                };
                float nr = h2r(p2, q, h + 1.0f/3.0f);
                float ng = h2r(p2, q, h);
                float nb = h2r(p2, q, h - 1.0f/3.0f);

                r = r + (nr - r) * match;
                g = g + (ng - g) * match;
                b = b + (nb - b) * match;
            }

            p[0] = static_cast<uint8_t>(std::clamp(r * 255.0f, 0.0f, 255.0f));
            p[1] = static_cast<uint8_t>(std::clamp(g * 255.0f, 0.0f, 255.0f));
            p[2] = static_cast<uint8_t>(std::clamp(b * 255.0f, 0.0f, 255.0f));
        }
    }
}

} // namespace FreeEffect
