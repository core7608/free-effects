#include "../effect_registry.h"
#include "noise_alpha_effect.h"
#include <algorithm>
#include <random>

namespace FreeEffect {

static EffectRegistrar<NoiseAlphaEffect> s_reg("Noise Alpha", "Stylize");

NoiseAlphaEffect::NoiseAlphaEffect() {
    addParameter(EffectParameter::makeDropdown("noiseType", "Noise Type", {"Uniform", "Squared"}, 0));
    addParameter(EffectParameter::makeFloat("amount", "Amount", 0.0, 100.0, 25.0));
    addParameter(EffectParameter::makeDropdown("alphaHandling", "Alpha Handling", {"Add", "Subtract", "Multiply"}, 0));
}

std::vector<ParameterGroup> NoiseAlphaEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeDropdown("noiseType", "Noise Type", {"Uniform", "Squared"}, 0),
        EffectParameter::makeFloat("amount", "Amount", 0.0, 100.0, false),
        EffectParameter::makeDropdown("alphaHandling", "Alpha Handling", {"Add", "Subtract", "Multiply"}, 0)
    }}};
}

std::unique_ptr<Effect> NoiseAlphaEffect::clone() const {
    auto e = std::make_unique<NoiseAlphaEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void NoiseAlphaEffect::render(PixelBuffer& buffer, double time) {
    int noiseType = getDropdownParam("noiseType");
    float amount = getFloatParam("amount") / 100.0f;
    int handling = getDropdownParam("alphaHandling");
    std::mt19937 rng(static_cast<unsigned>(time * 1000));
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            float noise = dist(rng);
            if (noiseType == 1) noise = (dist(rng) > 0.5f) ? 1.0f : 0.0f;
            float alpha = p[3] / 255.0f;
            switch (handling) {
                case 0: alpha = std::clamp(alpha + noise * amount, 0.0f, 1.0f); break;
                case 1: alpha = std::clamp(alpha - noise * amount, 0.0f, 1.0f); break;
                case 2: alpha = std::clamp(alpha * (1.0f - noise * amount), 0.0f, 1.0f); break;
            }
            p[3] = static_cast<uint8_t>(alpha * 255.0f);
        }
    }
}

} // namespace FreeEffect
