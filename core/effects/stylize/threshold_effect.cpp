#include "../effect_registry.h"
#include "threshold_effect.h"
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<ThresholdEffect> s_reg("Threshold", "Stylize");

ThresholdEffect::ThresholdEffect() {
    addParameter(EffectParameter::makeFloat("threshold", "Level", 0.0, 255.0, 128.0));
}

std::vector<ParameterGroup> ThresholdEffect::getParameterGroups() const {
    return {{getName(), {EffectParameter::makeFloat("threshold", "Level", 0.0, 255.0, false)}}};
}

std::unique_ptr<Effect> ThresholdEffect::clone() const {
    auto e = std::make_unique<ThresholdEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void ThresholdEffect::render(PixelBuffer& buffer, double time) {
    float thresh = getFloatParam("threshold");

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            float lum = p[0] * 0.299f + p[1] * 0.587f + p[2] * 0.114f;
            uint8_t val = (lum >= thresh) ? 255 : 0;
            p[0] = val; p[1] = val; p[2] = val;
        }
    }
}

} // namespace FreeEffect
