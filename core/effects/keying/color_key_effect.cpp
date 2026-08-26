#include "../effect_registry.h"
#include "color_key_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<ColorKeyEffect> s_reg("Color Key", "Keying");

ColorKeyEffect::ColorKeyEffect() {
    addParameter(EffectParameter::makeColor("keyColor", "Key Color", {0.0, 255.0, 0.0, 1.0}));
    addParameter(EffectParameter::makeFloat("tolerance", "Tolerance", 0.0, 255.0, 25.0));
    addParameter(EffectParameter::makeFloat("edgeFeather", "Edge Feather", 0.0, 100.0, 0.0));
    addParameter(EffectParameter::makeFloat("edgeThin", "Edge Thin", -100.0, 100.0, 0.0));
    addParameter(EffectParameter::makeFloat("spill", "Spill", 0.0, 100.0, 0.0));
}

std::vector<ParameterGroup> ColorKeyEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeColor("keyColor", "Key Color", {0.0, 255.0, 0.0, 1.0}),
        EffectParameter::makeFloat("tolerance", "Tolerance", 0.0, 255.0, false),
        EffectParameter::makeFloat("edgeFeather", "Edge Feather", 0.0, 100.0, false),
        EffectParameter::makeFloat("edgeThin", "Edge Thin", -100.0, 100.0, false),
        EffectParameter::makeFloat("spill", "Spill", 0.0, 100.0, false)
    }}};
}

std::unique_ptr<Effect> ColorKeyEffect::clone() const {
    auto e = std::make_unique<ColorKeyEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void ColorKeyEffect::render(PixelBuffer& buffer, double time) {
    Color key = getColorParam("keyColor");
    float tolerance = getFloatParam("tolerance");
    float feather = getFloatParam("edgeFeather");
    float spill = getFloatParam("spill") / 100.0f;

    float kr = key.r, kg = key.g, kb = key.b;

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            float dr = p[0] - kr;
            float dg = p[1] - kg;
            float db = p[2] - kb;
            float dist = std::sqrt(dr * dr + dg * dg + db * db);

            float alpha;
            if (dist <= tolerance - feather) {
                alpha = 0.0f;
            } else if (dist <= tolerance + feather) {
                alpha = (dist - (tolerance - feather)) / (2.0f * feather);
            } else {
                alpha = 1.0f;
            }

            if (spill > 0 && alpha < 1.0f) {
                float maxC = std::max({p[0], p[1], p[2]});
                float minC = std::min({p[0], p[1], p[2]});
                float sat = (maxC > 0) ? (maxC - minC) / maxC : 0;
                if (sat > 0.1f) {
                    float desat = sat * spill * (1.0f - alpha);
                    float avg = (p[0] + p[1] + p[2]) / 3.0f;
                    p[0] = static_cast<uint8_t>(std::clamp(static_cast<double>(p[0] + (avg - p[0]) * desat), 0.0, 255.0));
                    p[1] = static_cast<uint8_t>(std::clamp(static_cast<double>(p[1] + (avg - p[1]) * desat), 0.0, 255.0));
                    p[2] = static_cast<uint8_t>(std::clamp(static_cast<double>(p[2] + (avg - p[2]) * desat), 0.0, 255.0));
                }
            }

            p[3] = static_cast<uint8_t>(alpha * 255.0f);
        }
    }
}

} // namespace FreeEffect
