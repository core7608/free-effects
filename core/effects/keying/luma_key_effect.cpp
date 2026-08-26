#include "../effect_registry.h"
#include "luma_key_effect.h"
#include <cmath>
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<LumaKeyEffect> s_reg("Luma Key", "Keying");

LumaKeyEffect::LumaKeyEffect() {
    addParameter(EffectParameter::makeInt("key_type", "Key Type", 0, 2, 0));
    addParameter(EffectParameter::makeFloat("low", "Low", 0.0, 1.0, 0.0));
    addParameter(EffectParameter::makeFloat("high", "High", 0.0, 1.0, 1.0));
    addParameter(EffectParameter::makeFloat("tolerance", "Tolerance", 0.0, 1.0, 0.1));
    addParameter(EffectParameter::makeFloat("feather", "Feather", 0.0, 1.0, 0.1));
}

std::vector<ParameterGroup> LumaKeyEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeInt("key_type", "Key Type", 0, 2, 0),
        EffectParameter::makeFloat("low", "Low", 0.0, 1.0, 0.0),
        EffectParameter::makeFloat("high", "High", 0.0, 1.0, 1.0),
        EffectParameter::makeFloat("tolerance", "Tolerance", 0.0, 1.0, 0.1),
        EffectParameter::makeFloat("feather", "Feather", 0.0, 1.0, 0.1)
    }}};
}

std::unique_ptr<Effect> LumaKeyEffect::clone() const {
    auto e = std::make_unique<LumaKeyEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void LumaKeyEffect::render(PixelBuffer& buffer, double time) {
    if (buffer.width == 0 || buffer.height == 0) return;
    int keyType = getIntParam("key_type");
    double low = getFloatParam("low");
    double high = getFloatParam("high");
    double tolerance = getFloatParam("tolerance");
    double feather = getFloatParam("feather");
    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            double luma = (p[0] * 0.299 + p[1] * 0.587 + p[2] * 0.114) / 255.0;
            double alpha = 1.0;
            switch (keyType) {
                case 0:
                    alpha = std::clamp((luma - low) / (tolerance + 0.001), 0.0, 1.0);
                    if (luma > high) alpha = 0.0;
                    break;
                case 1:
                    alpha = std::clamp((high - luma) / (tolerance + 0.001), 0.0, 1.0);
                    if (luma < low) alpha = 0.0;
                    break;
                case 2:
                    if (luma >= low && luma <= high) alpha = 0.0;
                    else alpha = 1.0;
                    break;
            }
            if (feather > 0.001 && alpha > 0.0 && alpha < 1.0) {
                double t = (alpha - 0.5) / (feather * 0.5 + 0.001) * 0.5 + 0.5;
                alpha = std::clamp(t, 0.0, 1.0);
            }
            p[3] = static_cast<uint8_t>(alpha * 255.0);
        }
    }
}

} // namespace FreeEffect
