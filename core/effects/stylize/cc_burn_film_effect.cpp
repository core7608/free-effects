#include "../effect_registry.h"
#include "cc_burn_film_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<CCBurnFilmEffect> s_reg("CC Burn Film", "Stylize");

CCBurnFilmEffect::CCBurnFilmEffect() {
    addParameter(EffectParameter::makeFloat("burn", "Burn", 0.0, 100.0, 0.0));
    addParameter(EffectParameter::makeFloat("center", "Center", 0.0, 100.0, 50.0));
    addParameter(EffectParameter::makeFloat("randomSeed", "Random Seed", 0.0, 100.0, 50.0));
}

std::unique_ptr<Effect> CCBurnFilmEffect::clone() const {
    auto e = std::make_unique<CCBurnFilmEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void CCBurnFilmEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    float burn = getFloatParam("burn") / 100.0f;
    float center = getFloatParam("center") / 100.0f;
    float seed = getFloatParam("randomSeed");

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            float hash = std::fmod(std::sin(static_cast<float>(x) * 12.9898f + y * 78.233f + seed) * 43758.5453f, 1.0f);
            float nx = static_cast<float>(x) / buffer.width - center;
            float ny = static_cast<float>(y) / buffer.height - 0.5f;
            float dist = std::sqrt(nx * nx + ny * ny);
            float burnAmount = burn * (1.0f - dist) * hash;
            if (burnAmount > 0.5f) {
                uint8_t* p = buffer.pixelAt(x, y);
                float fire = (burnAmount - 0.5f) * 2.0f;
                p[0] = static_cast<uint8_t>(std::min(255.0, static_cast<double>(p[0] + 255.0f * fire)));
                p[1] = static_cast<uint8_t>(std::clamp(static_cast<double>(p[1] * (1.0f - fire * 0.5f)), 0.0, 255.0));
                p[2] = static_cast<uint8_t>(std::clamp(static_cast<double>(p[2] * (1.0f - fire * 0.8f)), 0.0, 255.0));
            }
        }
    }
}

} // namespace FreeEffect
