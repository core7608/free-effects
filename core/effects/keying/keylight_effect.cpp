#include "../effect_registry.h"
#include "keylight_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<KeylightEffect> s_reg("Keylight", "Keying");

KeylightEffect::KeylightEffect() {
    addParameter(EffectParameter::makeColor("screenColor", "Screen Color", {0.0, 255.0, 0.0, 1.0}));
    addParameter(EffectParameter::makeFloat("screenGain", "Screen Gain", 0.0, 200.0, 100.0));
    addParameter(EffectParameter::makeFloat("screenBalance", "Screen Balance", 0.0, 100.0, 50.0));
    addParameter(EffectParameter::makeDropdown("screenMatte", "Screen Matte", {"Combined Matte", "Screen Matte", "Inside Matte", "Outside Matte"}, 0));
    addParameter(EffectParameter::makeFloat("clipBlack", "Clip Black", 0.0, 255.0, 0.0));
    addParameter(EffectParameter::makeFloat("clipWhite", "Clip White", 0.0, 255.0, 255.0));
    addParameter(EffectParameter::makeFloat("screenSoftness", "Screen Softness", -100.0, 100.0, 0.0));
    addParameter(EffectParameter::makeBool("despill", "Despill", true));
}

std::unique_ptr<Effect> KeylightEffect::clone() const {
    auto e = std::make_unique<KeylightEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void KeylightEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    Color sc = getColorParam("screenColor");
    float gain = getFloatParam("screenGain") / 100.0f;
    float balance = getFloatParam("screenBalance") / 100.0f;
    float clipB = getFloatParam("clipBlack");
    float clipW = getFloatParam("clipWhite");
    float soft = getFloatParam("screenSoftness") / 100.0f;
    bool despill = getBoolParam("despill");

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            float r = p[0], g = p[1], b = p[2];
            float dr = (r - sc.r * gain);
            float dg = (g - sc.g * gain);
            float db = (b - sc.b * gain);
            float matte = 1.0f - std::clamp((std::abs(dr) + std::abs(dg) + std::abs(db)) / 765.0f, 0.0f, 1.0f);
            matte = std::clamp((matte - clipB / 255.0f) / (clipW / 255.0f - clipB / 255.0f + 0.001f), 0.0f, 1.0f);

            if (despill && matte < 1.0f) {
                float luma = 0.299f * r + 0.587f * g + 0.114f * b;
                float spill = std::max({sc.r, sc.g, sc.b}) > 0 ? std::min({r / std::max(sc.r, 1.0), g / std::max(sc.g, 1.0), b / std::max(sc.b, 1.0)}) : 0;
                p[0] = static_cast<uint8_t>(std::clamp(r * (1.0f - (1.0f - matte) * spill), 0.0f, 255.0f));
                p[1] = static_cast<uint8_t>(std::clamp(g * (1.0f - (1.0f - matte) * spill), 0.0f, 255.0f));
                p[2] = static_cast<uint8_t>(std::clamp(b * (1.0f - (1.0f - matte) * spill), 0.0f, 255.0f));
            }
            p[3] = static_cast<uint8_t>(std::clamp(matte * 255.0f, 0.0f, 255.0f));
        }
    }
}

} // namespace FreeEffect
