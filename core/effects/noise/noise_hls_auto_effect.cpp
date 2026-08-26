#include "../effect_registry.h"
#include "noise_hls_auto_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<NoiseHLSAutoEffect> s_reg("Noise HLS Auto", "Noise & Grain");

NoiseHLSAutoEffect::NoiseHLSAutoEffect() {
    addParameter(EffectParameter::makeFloat("noise", "Noise", 0.0, 100.0, 10.0));
    addParameter(EffectParameter::makeDropdown("noiseType", "Noise Type", {"Uniform", "Grainy", "Soft"}, 0));
    addParameter(EffectParameter::makeBool("animateNoise", "Animate Noise", true));
}

std::unique_ptr<Effect> NoiseHLSAutoEffect::clone() const {
    auto e = std::make_unique<NoiseHLSAutoEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void NoiseHLSAutoEffect::render(PixelBuffer& buffer, double time) {
    float noise = getFloatParam("noise") / 100.0f;
    bool animate = getBoolParam("animateNoise");
    float seed = animate ? static_cast<float>(time * 10.0) : 0.0f;

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            float n = (std::fmod(std::sin((x+seed)*12.9898f+y*78.233f)*43758.5453f, 1.0f) - 0.5f) * noise * 2.0f;
            p[0] = static_cast<uint8_t>(std::clamp(static_cast<double>(p[0] + n * 255.0f), 0.0, 255.0));
            p[1] = static_cast<uint8_t>(std::clamp(static_cast<double>(p[1] + n * 255.0f), 0.0, 255.0));
            p[2] = static_cast<uint8_t>(std::clamp(static_cast<double>(p[2] + n * 255.0f), 0.0, 255.0));
        }
    }
}

} // namespace FreeEffect
