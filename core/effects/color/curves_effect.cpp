#include "../effect_registry.h"
#include "curves_effect.h"
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<CurvesEffect> s_reg("Curves", "Color Correction");

CurvesEffect::CurvesEffect() {
    for (auto& ch : m_lut) {
        for (int i = 0; i < 256; i++) ch[i] = static_cast<uint8_t>(i);
    }
}

std::vector<ParameterGroup> CurvesEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeFloat("rgbPreset", "RGB Preset", 0.0, 2.0, 1.0),
        EffectParameter::makeFloat("rPreset", "Red Preset", 0.0, 2.0, 1.0),
        EffectParameter::makeFloat("gPreset", "Green Preset", 0.0, 2.0, 1.0),
        EffectParameter::makeFloat("bPreset", "Blue Preset", 0.0, 2.0, 1.0)
    }}};
}

std::unique_ptr<Effect> CurvesEffect::clone() const {
    auto e = std::make_unique<CurvesEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void CurvesEffect::buildLUT(int channel) {
    float preset = 1.0f;
    if (channel == 0) preset = getFloatParam("rgbPreset");
    else if (channel == 1) preset = getFloatParam("rPreset");
    else if (channel == 2) preset = getFloatParam("gPreset");
    else if (channel == 3) preset = getFloatParam("bPreset");

    for (int i = 0; i < 256; i++) {
        float v = static_cast<float>(i) / 255.0f;
        v = std::pow(v, 1.0f / std::max(preset, 0.01f));
        m_lut[channel][i] = static_cast<uint8_t>(std::clamp(static_cast<double>(v * 255.0f), 0.0, 255.0));
    }
}

void CurvesEffect::render(PixelBuffer& buffer, double time) {
    for (int ch = 0; ch < 4; ch++) buildLUT(ch);

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            p[0] = m_lut[1][p[0]];
            p[1] = m_lut[2][p[1]];
            p[2] = m_lut[3][p[2]];
        }
    }
}

} // namespace FreeEffect
