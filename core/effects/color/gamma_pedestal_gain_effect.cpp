#include "../effect_registry.h"
#include "gamma_pedestal_gain_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<GammaPedestalGainEffect> s_reg("Gamma/Pedestal/Gain", "Color Correction");

GammaPedestalGainEffect::GammaPedestalGainEffect() {
    addParameter(EffectParameter::makeColor("blackStretch", "Black Stretch", {1.0, 1.0, 1.0, 1.0}));
    addParameter(EffectParameter::makeColor("gamma", "Gamma", {1.0, 1.0, 1.0, 1.0}));
    addParameter(EffectParameter::makeColor("pedestal", "Pedestal", {0.0, 0.0, 0.0, 1.0}));
    addParameter(EffectParameter::makeColor("gain", "Gain", {1.0, 1.0, 1.0, 1.0}));
}

std::unique_ptr<Effect> GammaPedestalGainEffect::clone() const {
    auto e = std::make_unique<GammaPedestalGainEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void GammaPedestalGainEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    Color bs = getColorParam("blackStretch");
    Color gamma = getColorParam("gamma");
    Color ped = getColorParam("pedestal");
    Color gain = getColorParam("gain");

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            float c[3] = {p[0] / 255.0f, p[1] / 255.0f, p[2] / 255.0f};
            float bsArr[3] = {static_cast<float>(bs.r), static_cast<float>(bs.g), static_cast<float>(bs.b)};
            float gamArr[3] = {std::max(static_cast<float>(gamma.r), 0.01f), std::max(static_cast<float>(gamma.g), 0.01f), std::max(static_cast<float>(gamma.b), 0.01f)};
            float pedArr[3] = {static_cast<float>(ped.r), static_cast<float>(ped.g), static_cast<float>(ped.b)};
            float gainArr[3] = {static_cast<float>(gain.r), static_cast<float>(gain.g), static_cast<float>(gain.b)};

            for (int ch = 0; ch < 3; ch++) {
                float v = std::pow(c[ch], 1.0f / gamArr[ch]);
                v = v * bsArr[ch];
                v = v + pedArr[ch] * 0.5f;
                v = v * gainArr[ch];
                p[ch] = static_cast<uint8_t>(std::clamp(v * 255.0f, 0.0f, 255.0f));
            }
        }
    }
}

} // namespace FreeEffect
