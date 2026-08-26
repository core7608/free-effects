#include "../effect_registry.h"
#include "hue_saturation_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<HueSaturationEffect> s_reg("Hue/Saturation", "Color Correction");

HueSaturationEffect::HueSaturationEffect() {
    addParameter(EffectParameter::makeAngle("hue", "Hue Rotation", 0.0));
    addParameter(EffectParameter::makeFloat("saturation", "Saturation", -100.0, 100.0, 0.0));
    addParameter(EffectParameter::makeFloat("lightness", "Lightness", -100.0, 100.0, 0.0));
}

std::vector<ParameterGroup> HueSaturationEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeAngle("hue", "Hue Rotation", 0.0),
        EffectParameter::makeFloat("saturation", "Saturation", -100.0, 100.0, false),
        EffectParameter::makeFloat("lightness", "Lightness", -100.0, 100.0, false)
    }}};
}

std::unique_ptr<Effect> HueSaturationEffect::clone() const {
    auto e = std::make_unique<HueSaturationEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void HueSaturationEffect::render(PixelBuffer& buffer, double time) {
    float hueShift = getFloatParam("hue") / 360.0f;
    float satAdj = 1.0f + getFloatParam("saturation") / 100.0f;
    float lightAdj = getFloatParam("lightness") / 100.0f;

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            float r = p[0] / 255.0f, g = p[1] / 255.0f, b = p[2] / 255.0f;

            float cmax = std::max({r, g, b});
            float cmin = std::min({r, g, b});
            float delta = cmax - cmin;
            float h = 0, s = 0, l = (cmax + cmin) / 2.0f;

            if (delta > 0.001f) {
                s = l > 0.5f ? delta / (2.0f - cmax - cmin) : delta / (cmax + cmin);
                if (cmax == r) h = (g - b) / delta + (g < b ? 6.0f : 0.0f);
                else if (cmax == g) h = (b - r) / delta + 2.0f;
                else h = (r - g) / delta + 4.0f;
                h /= 6.0f;
            }

            h = std::fmod(h + hueShift + 1.0f, 1.0f);
            s = std::clamp(s * satAdj, 0.0f, 1.0f);
            l = std::clamp(l + lightAdj, 0.0f, 1.0f);

            float q = l < 0.5f ? l * (1.0f + s) : l + s - l * s;
            float p2 = 2.0f * l - q;
            auto hue2rgb = [](float p2, float q2, float t) -> float {
                if (t < 0) t += 1.0f;
                if (t > 1) t -= 1.0f;
                if (t < 1.0f/6.0f) return p2 + (q2 - p2) * 6.0f * t;
                if (t < 0.5f) return q2;
                if (t < 2.0f/3.0f) return p2 + (q2 - p2) * (2.0f/3.0f - t) * 6.0f;
                return p2;
            };
            p[0] = static_cast<uint8_t>(std::clamp(hue2rgb(p2, q, h + 1.0f/3.0f) * 255.0f, 0.0f, 255.0f));
            p[1] = static_cast<uint8_t>(std::clamp(hue2rgb(p2, q, h) * 255.0f, 0.0f, 255.0f));
            p[2] = static_cast<uint8_t>(std::clamp(hue2rgb(p2, q, h - 1.0f/3.0f) * 255.0f, 0.0f, 255.0f));
        }
    }
}

} // namespace FreeEffect
