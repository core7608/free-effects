#include "../effect_registry.h"
#include "posterize_time_effect.h"
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<PosterizeTimeEffect> s_reg("Posterize Time", "Utility");

PosterizeTimeEffect::PosterizeTimeEffect() {
    addParameter(EffectParameter::makeFloat("frameRate", "Frame Rate", 1.0, 120.0, 12.0));
}

std::vector<ParameterGroup> PosterizeTimeEffect::getParameterGroups() const {
    return {{getName(), {EffectParameter::makeFloat("frameRate", "Frame Rate", 1.0, 120.0, false)}}};
}

std::unique_ptr<Effect> PosterizeTimeEffect::clone() const {
    auto e = std::make_unique<PosterizeTimeEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void PosterizeTimeEffect::render(PixelBuffer& buffer, double time) {
    float frameRate = getFloatParam("frameRate");
    if (frameRate <= 0) frameRate = 1.0f;
    float frameDuration = 1.0f / frameRate;
    float posterizedTime = std::floor(time / frameDuration) * frameDuration;

    if (std::abs(time - posterizedTime) > 0.001f) {
        for (int y = 0; y < buffer.height; y++) {
            for (int x = 0; x < buffer.width; x++) {
                uint8_t* p = buffer.pixelAt(x, y);
                p[3] = 0;
            }
        }
    }
}

} // namespace FreeEffect
