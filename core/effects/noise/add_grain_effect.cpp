#include "../effect_registry.h"
#include "add_grain_effect.h"
#include <algorithm>
#include <random>

namespace FreeEffect {

static EffectRegistrar<AddGrainEffect> s_reg("Add Grain", "Noise & Grain");

AddGrainEffect::AddGrainEffect() {
    addParameter(EffectParameter::makeFloat("intensity", "Intensity", 0.0, 10.0, 1.0));
    addParameter(EffectParameter::makeFloat("size", "Size", 0.5, 5.0, 1.0));
    addParameter(EffectParameter::makeFloat("softness", "Softness", 0.0, 10.0, 2.0));
    addParameter(EffectParameter::makeBool("monochromatic", "Monochromatic", false));
}

std::unique_ptr<Effect> AddGrainEffect::clone() const {
    auto e = std::make_unique<AddGrainEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void AddGrainEffect::render(PixelBuffer& buffer, double time) {
    double intensity = getFloatParam("intensity");
    bool mono = getBoolParam("monochromatic");

    if (intensity <= 0.0) return;

    std::mt19937 gen(static_cast<unsigned int>(time * 1000.0));
    std::uniform_real_distribution<double> dist(-1.0, 1.0);

    double scale = intensity * 25.5;

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            if (mono) {
                double n = dist(gen) * scale;
                p[0] = static_cast<uint8_t>(std::clamp(p[0] + n, 0.0, 255.0));
                p[1] = static_cast<uint8_t>(std::clamp(p[1] + n, 0.0, 255.0));
                p[2] = static_cast<uint8_t>(std::clamp(p[2] + n, 0.0, 255.0));
            } else {
                p[0] = static_cast<uint8_t>(std::clamp(p[0] + dist(gen) * scale, 0.0, 255.0));
                p[1] = static_cast<uint8_t>(std::clamp(p[1] + dist(gen) * scale, 0.0, 255.0));
                p[2] = static_cast<uint8_t>(std::clamp(p[2] + dist(gen) * scale, 0.0, 255.0));
            }
        }
    }
}

} // namespace FreeEffect
