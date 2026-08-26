#include "../effect_registry.h"
#include "noise_effect.h"
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<NoiseEffect> s_reg("Noise", "Stylize");

NoiseEffect::NoiseEffect() {
    addParameter(EffectParameter::makeFloat("amount", "Amount", 0.0, 100.0, 10.0));
    addParameter(EffectParameter::makeBool("useColorNoise", "Use Color Noise", false));
}

std::vector<ParameterGroup> NoiseEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeFloat("amount", "Amount", 0.0, 100.0, false),
        EffectParameter::makeBool("useColorNoise", "Use Color Noise", false)
    }}};
}

std::unique_ptr<Effect> NoiseEffect::clone() const {
    auto e = std::make_unique<NoiseEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void NoiseEffect::render(PixelBuffer& buffer, double time) {
    float amount = getFloatParam("amount") * 2.55f;
    bool colorNoise = getBoolParam("useColorNoise");
    std::mt19937 rng(static_cast<unsigned>(time * 1000));
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            float noise = dist(rng) * amount;
            if (colorNoise) {
                p[0] = static_cast<uint8_t>(std::clamp(static_cast<double>(p[0] + dist(rng) * amount), 0.0, 255.0));
                p[1] = static_cast<uint8_t>(std::clamp(static_cast<double>(p[1] + dist(rng) * amount), 0.0, 255.0));
                p[2] = static_cast<uint8_t>(std::clamp(static_cast<double>(p[2] + dist(rng) * amount), 0.0, 255.0));
            } else {
                float n = noise;
                p[0] = static_cast<uint8_t>(std::clamp(static_cast<double>(p[0] + n), 0.0, 255.0));
                p[1] = static_cast<uint8_t>(std::clamp(static_cast<double>(p[1] + n), 0.0, 255.0));
                p[2] = static_cast<uint8_t>(std::clamp(static_cast<double>(p[2] + n), 0.0, 255.0));
            }
        }
    }
}

} // namespace FreeEffect
