#include "../effect_registry.h"
#include "exposure_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<ExposureEffect> s_reg("Exposure", "Color Correction");

ExposureEffect::ExposureEffect() {
    addParameter(EffectParameter::makeFloat("exposure", "Exposure", -10.0, 10.0, 0.0));
    addParameter(EffectParameter::makeFloat("offset", "Offset", -1.0, 1.0, 0.0));
    addParameter(EffectParameter::makeFloat("gamma", "Gamma Correction", 0.1, 10.0, 1.0));
}

std::vector<ParameterGroup> ExposureEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeFloat("exposure", "Exposure", -10.0, 10.0, false),
        EffectParameter::makeFloat("offset", "Offset", -1.0, 1.0, false),
        EffectParameter::makeFloat("gamma", "Gamma Correction", 0.1, 10.0, false)
    }}};
}

std::unique_ptr<Effect> ExposureEffect::clone() const {
    auto e = std::make_unique<ExposureEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void ExposureEffect::render(PixelBuffer& buffer, double time) {
    float exp = getFloatParam("exposure");
    float offset = getFloatParam("offset") * 255.0f;
    float gamma = getFloatParam("gamma");
    float pow2 = std::pow(2.0f, exp);
    float invGamma = 1.0f / std::max(gamma, 0.01f);

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            for (int c = 0; c < 3; c++) {
                float v = static_cast<float>(p[c]);
                v = std::pow(v / 255.0f, invGamma) * 255.0f;
                v = v * pow2 + offset;
                p[c] = static_cast<uint8_t>(std::clamp(static_cast<double>(v), 0.0, 255.0));
            }
        }
    }
}

} // namespace FreeEffect
