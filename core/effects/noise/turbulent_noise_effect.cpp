#include "../effect_registry.h"
#include "turbulent_noise_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<TurbulentNoiseEffect> s_reg("Turbulent Noise", "Noise & Grain");

TurbulentNoiseEffect::TurbulentNoiseEffect() {
    addParameter(EffectParameter::makeInt("complexity", "Complexity", 1, 10, 6));
    addParameter(EffectParameter::makeFloat("evolution", "Evolution", 0.0, 360.0, 0.0));
    addParameter(EffectParameter::makeFloat("scale", "Scale", 1.0, 1000.0, 100.0));
    addParameter(EffectParameter::makeVec2("offset", "Offset", {0.0, 0.0}));
    addParameter(EffectParameter::makeDropdown("noiseType", "Noise Type", {"Turbulent Smooth", "Turbulent Basic", "Turbulent Sharp", "Dynamic"}, 1));
}

std::unique_ptr<Effect> TurbulentNoiseEffect::clone() const {
    auto e = std::make_unique<TurbulentNoiseEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void TurbulentNoiseEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    int complexity = getIntParam("complexity");
    float evolution = getFloatParam("evolution") * 3.14159265f / 180.0f;
    float scale = getFloatParam("scale");
    Vec2 offset = getVec2Param("offset");

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            float nx = (x + offset.x) / scale;
            float ny = (y + offset.y) / scale;
            float val = 0, amp = 1.0f, freq = 1.0f, maxVal = 0;
            for (int o = 0; o < complexity; o++) {
                float v = std::sin(nx * freq + evolution) * std::cos(ny * freq + evolution * 0.7f);
                v += std::sin(nx * freq * 2.1f + ny * freq * 1.7f + evolution * 1.3f) * 0.5f;
                val += v * amp;
                maxVal += amp;
                amp *= 0.5f;
                freq *= 2.0f;
            }
            val = (val / maxVal + 1.0f) * 0.5f;
            uint8_t* p = buffer.pixelAt(x, y);
            p[0] = static_cast<uint8_t>(std::clamp(val * 255.0f, 0.0f, 255.0f));
            p[1] = p[0]; p[2] = p[0];
        }
    }
}

} // namespace FreeEffect
