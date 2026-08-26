#include "../effect_registry.h"
#include "linear_color_key_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<LinearColorKeyEffect> s_reg("Linear Color Key", "Keying");

LinearColorKeyEffect::LinearColorKeyEffect() {
    addParameter(EffectParameter::makeColor("keyColor", "Key Color", {0.0, 255.0, 0.0, 1.0}));
    addParameter(EffectParameter::makeFloat("tolerance", "Tolerance", 0.0, 255.0, 30.0));
    addParameter(EffectParameter::makeFloat("softness", "Softness", 0.0, 100.0, 10.0));
}

std::vector<ParameterGroup> LinearColorKeyEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeColor("keyColor", "Key Color", {0.0, 255.0, 0.0, 1.0}),
        EffectParameter::makeFloat("tolerance", "Tolerance", 0.0, 255.0, false),
        EffectParameter::makeFloat("softness", "Softness", 0.0, 100.0, false)
    }}};
}

std::unique_ptr<Effect> LinearColorKeyEffect::clone() const {
    auto e = std::make_unique<LinearColorKeyEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void LinearColorKeyEffect::render(PixelBuffer& buffer, double time) {
    Color key = getColorParam("keyColor");
    float tolerance = getFloatParam("tolerance");
    float softness = getFloatParam("softness");

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            float dr = p[0] - key.r;
            float dg = p[1] - key.g;
            float db = p[2] - key.b;
            float dist = std::abs(dr) + std::abs(dg) + std::abs(db);

            float alpha;
            if (dist <= tolerance - softness) {
                alpha = 0.0f;
            } else if (dist <= tolerance + softness && softness > 0) {
                alpha = (dist - (tolerance - softness)) / (2.0f * softness);
            } else {
                alpha = 1.0f;
            }
            alpha = std::clamp(alpha, 0.0f, 1.0f);
            p[3] = static_cast<uint8_t>(alpha * 255.0f);
        }
    }
}

} // namespace FreeEffect
