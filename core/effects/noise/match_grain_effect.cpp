#include "../effect_registry.h"
#include "match_grain_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<MatchGrainEffect> s_reg("Match Grain", "Noise & Grain");

MatchGrainEffect::MatchGrainEffect() {
    addParameter(EffectParameter::makeFloat("intensity", "Intensity", 0.0, 100.0, 50.0));
    addParameter(EffectParameter::makeInt("grainSize", "Grain Size", 1, 10, 2));
    addParameter(EffectParameter::makeBool("animate", "Animate", false));
}

std::unique_ptr<Effect> MatchGrainEffect::clone() const {
    auto e = std::make_unique<MatchGrainEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void MatchGrainEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    float intensity = getFloatParam("intensity") / 100.0f;
    int grainSize = getIntParam("grainSize");
    float seed = getBoolParam("animate") ? static_cast<float>(time * 24.0) : 0.0f;

    for (int y = 0; y < buffer.height; y += grainSize) {
        for (int x = 0; x < buffer.width; x += grainSize) {
            float g = (std::fmod(std::sin((x+seed)*12.9898f+(y+seed)*78.233f)*43758.5453f, 1.0f) - 0.5f) * intensity * 2.0f;
            for (int dy = 0; dy < grainSize && y+dy < buffer.height; dy++) {
                for (int dx = 0; dx < grainSize && x+dx < buffer.width; dx++) {
                    uint8_t* p = buffer.pixelAt(x+dx, y+dy);
                    float luma = 0.299f * p[0] + 0.587f * p[1] + 0.114f * p[2];
                    float factor = 1.0f + g * (luma > 128 ? 1.0f : 0.5f);
                    p[0] = static_cast<uint8_t>(std::clamp(static_cast<double>(p[0] * factor), 0.0, 255.0));
                    p[1] = static_cast<uint8_t>(std::clamp(static_cast<double>(p[1] * factor), 0.0, 255.0));
                    p[2] = static_cast<uint8_t>(std::clamp(static_cast<double>(p[2] * factor), 0.0, 255.0));
                }
            }
        }
    }
}

} // namespace FreeEffect
