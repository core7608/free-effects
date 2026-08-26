#include "../effect_registry.h"
#include "color_balance_rgb_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<ColorBalanceRGBEffect> s_reg("Color Balance (RGB)", "Color Correction");

ColorBalanceRGBEffect::ColorBalanceRGBEffect() {
    addParameter(EffectParameter::makeFloat("shadowRed", "Shadows Red", -100.0, 100.0, 0.0));
    addParameter(EffectParameter::makeFloat("shadowGreen", "Shadows Green", -100.0, 100.0, 0.0));
    addParameter(EffectParameter::makeFloat("shadowBlue", "Shadows Blue", -100.0, 100.0, 0.0));
    addParameter(EffectParameter::makeFloat("midtoneRed", "Midtones Red", -100.0, 100.0, 0.0));
    addParameter(EffectParameter::makeFloat("midtoneGreen", "Midtones Green", -100.0, 100.0, 0.0));
    addParameter(EffectParameter::makeFloat("midtoneBlue", "Midtones Blue", -100.0, 100.0, 0.0));
    addParameter(EffectParameter::makeFloat("highlightRed", "Highlights Red", -100.0, 100.0, 0.0));
    addParameter(EffectParameter::makeFloat("highlightGreen", "Highlights Green", -100.0, 100.0, 0.0));
    addParameter(EffectParameter::makeFloat("highlightBlue", "Highlights Blue", -100.0, 100.0, 0.0));
    addParameter(EffectParameter::makeBool("preserveLuminosity", "Preserve Luminosity", true));
}

std::unique_ptr<Effect> ColorBalanceRGBEffect::clone() const {
    auto e = std::make_unique<ColorBalanceRGBEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void ColorBalanceRGBEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    float sR = getFloatParam("shadowRed") / 100.0f, sG = getFloatParam("shadowGreen") / 100.0f, sB = getFloatParam("shadowBlue") / 100.0f;
    float mR = getFloatParam("midtoneRed") / 100.0f, mG = getFloatParam("midtoneGreen") / 100.0f, mB = getFloatParam("midtoneBlue") / 100.0f;
    float hR = getFloatParam("highlightRed") / 100.0f, hG = getFloatParam("highlightGreen") / 100.0f, hB = getFloatParam("highlightBlue") / 100.0f;
    bool preserveLum = getBoolParam("preserveLuminosity");

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            float r = p[0] / 255.0f, g = p[1] / 255.0f, b = p[2] / 255.0f;
            float luma = 0.299f * r + 0.587f * g + 0.114f * b;

            float shadow = std::max(0.0f, 1.0f - luma * 2.5f);
            float mid = std::max(0.0f, 1.0f - std::abs(luma - 0.5f) * 3.0f);
            float highlight = std::max(0.0f, luma * 2.5f - 1.5f);

            r += sR * shadow + mR * mid + hR * highlight;
            g += sG * shadow + mG * mid + hG * highlight;
            b += sB * shadow + mB * mid + hB * highlight;

            if (preserveLum) {
                float newLuma = 0.299f * r + 0.587f * g + 0.114f * b;
                if (newLuma > 0.001f) {
                    float scale = luma / newLuma;
                    r *= scale; g *= scale; b *= scale;
                }
            }

            p[0] = static_cast<uint8_t>(std::clamp(r * 255.0f, 0.0f, 255.0f));
            p[1] = static_cast<uint8_t>(std::clamp(g * 255.0f, 0.0f, 255.0f));
            p[2] = static_cast<uint8_t>(std::clamp(b * 255.0f, 0.0f, 255.0f));
        }
    }
}

} // namespace FreeEffect
