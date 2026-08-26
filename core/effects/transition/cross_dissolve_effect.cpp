#include "../effect_registry.h"
#include "cross_dissolve_effect.h"
#include <cmath>
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<CrossDissolveEffect> s_reg("Cross Dissolve", "Transition");

CrossDissolveEffect::CrossDissolveEffect() {
    addParameter(EffectParameter::makeFloat("progress", "Progress", 0.0, 1.0, 0.0));
    addParameter(EffectParameter::makeBool("use_noise", "Dither", true));
}

std::vector<ParameterGroup> CrossDissolveEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeFloat("progress", "Progress", 0.0, 1.0, 0.0),
        EffectParameter::makeBool("use_noise", "Dither", true)
    }}};
}

std::unique_ptr<Effect> CrossDissolveEffect::clone() const {
    auto e = std::make_unique<CrossDissolveEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void CrossDissolveEffect::render(PixelBuffer& buffer, double time) {
    if (buffer.width == 0 || buffer.height == 0) return;
    double progress = getFloatParam("progress");
    bool dither = getBoolParam("use_noise");
    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            double threshold = 0.0;
            if (dither) {
                double seed = std::sin(x * 127.1 + y * 311.7) * 43758.5453;
                threshold = seed - std::floor(seed);
            }
            double fade = std::clamp((progress - threshold * 0.1) / (1.0 - 0.1 * dither), 0.0, 1.0);
            if (fade < 0.001) continue;
            uint8_t* p = buffer.pixelAt(x, y);
            p[0] = static_cast<uint8_t>(p[0] * (1.0 - fade));
            p[1] = static_cast<uint8_t>(p[1] * (1.0 - fade));
            p[2] = static_cast<uint8_t>(p[2] * (1.0 - fade));
            p[3] = static_cast<uint8_t>(p[3] * (1.0 - fade));
        }
    }
}

} // namespace FreeEffect
