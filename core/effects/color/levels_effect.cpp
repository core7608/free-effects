#include "../effect_registry.h"
#include "levels_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<LevelsEffect> s_reg("Levels", "Color Correction");

LevelsEffect::LevelsEffect() {
    addParameter(EffectParameter::makeFloat("inputBlack", "Input Black", 0.0, 255.0, 0.0));
    addParameter(EffectParameter::makeFloat("inputWhite", "Input White", 0.0, 255.0, 255.0));
    addParameter(EffectParameter::makeFloat("gamma", "Gamma", 0.1, 10.0, 1.0));
    addParameter(EffectParameter::makeFloat("outputBlack", "Output Black", 0.0, 255.0, 0.0));
    addParameter(EffectParameter::makeFloat("outputWhite", "Output White", 0.0, 255.0, 255.0));
}

std::vector<ParameterGroup> LevelsEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeFloat("inputBlack", "Input Black", 0.0, 255.0, false),
        EffectParameter::makeFloat("inputWhite", "Input White", 0.0, 255.0, false),
        EffectParameter::makeFloat("gamma", "Gamma", 0.1, 10.0, false),
        EffectParameter::makeFloat("outputBlack", "Output Black", 0.0, 255.0, false),
        EffectParameter::makeFloat("outputWhite", "Output White", 0.0, 255.0, false)
    }}};
}

std::unique_ptr<Effect> LevelsEffect::clone() const {
    auto e = std::make_unique<LevelsEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void LevelsEffect::render(PixelBuffer& buffer, double time) {
    float inBlack = getFloatParam("inputBlack");
    float inWhite = getFloatParam("inputWhite");
    float gamma = getFloatParam("gamma");
    float outBlack = getFloatParam("outputBlack");
    float outWhite = getFloatParam("outputWhite");

    if (inWhite <= inBlack) inWhite = inBlack + 1.0f;
    float range = inWhite - inBlack;
    float invGamma = 1.0f / std::max(gamma, 0.01f);

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            for (int c = 0; c < 3; c++) {
                float v = static_cast<float>(p[c]);
                v = std::clamp((v - inBlack) / range, 0.0f, 1.0f);
                v = std::pow(v, invGamma);
                v = outBlack + v * (outWhite - outBlack);
                p[c] = static_cast<uint8_t>(std::clamp(static_cast<double>(v), 0.0, 255.0));
            }
        }
    }
}

} // namespace FreeEffect
