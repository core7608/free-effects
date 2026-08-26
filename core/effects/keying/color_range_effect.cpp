#include "../effect_registry.h"
#include "color_range_effect.h"
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<ColorRangeEffect> s_reg("Color Range", "Keying");

ColorRangeEffect::ColorRangeEffect() {
    addParameter(EffectParameter::makeColor("keyColor", "Key Color", {0.0, 255.0, 0.0, 1.0}));
    addParameter(EffectParameter::makeFloat("fuzziness", "Fuzziness", 0.0, 255.0, 20.0));
}

std::vector<ParameterGroup> ColorRangeEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeColor("keyColor", "Key Color", {0.0, 255.0, 0.0, 1.0}),
        EffectParameter::makeFloat("fuzziness", "Fuzziness", 0.0, 255.0, false)
    }}};
}

std::unique_ptr<Effect> ColorRangeEffect::clone() const {
    auto e = std::make_unique<ColorRangeEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void ColorRangeEffect::render(PixelBuffer& buffer, double time) {
    Color key = getColorParam("keyColor");
    float fuzziness = getFloatParam("fuzziness");

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            float dr = p[0] - key.r;
            float dg = p[1] - key.g;
            float db = p[2] - key.b;
            float dist = std::sqrt(dr * dr + dg * dg + db * db);
            float alpha = std::clamp(dist / std::max(fuzziness, 0.001f), 0.0f, 1.0f);
            p[3] = static_cast<uint8_t>(alpha * 255.0f);
        }
    }
}

} // namespace FreeEffect
