#include "../effect_registry.h"
#include "lumetri_color_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<LumetriColorEffect> s_reg("Lumetri Color", "Color Correction");

LumetriColorEffect::LumetriColorEffect() {
    addParameter(EffectParameter::makeFloat("temperature", "Temperature", -100.0, 100.0, 0.0));
    addParameter(EffectParameter::makeFloat("tint", "Tint", -100.0, 100.0, 0.0));
    addParameter(EffectParameter::makeFloat("exposure", "Exposure", -5.0, 5.0, 0.0));
    addParameter(EffectParameter::makeFloat("contrast", "Contrast", -100.0, 100.0, 0.0));
    addParameter(EffectParameter::makeFloat("highlights", "Highlights", -100.0, 100.0, 0.0));
    addParameter(EffectParameter::makeFloat("shadows", "Shadows", -100.0, 100.0, 0.0));
    addParameter(EffectParameter::makeFloat("whites", "Whites", -100.0, 100.0, 0.0));
    addParameter(EffectParameter::makeFloat("blacks", "Blacks", -100.0, 100.0, 0.0));
    addParameter(EffectParameter::makeFloat("saturation", "Saturation", 0.0, 200.0, 100.0));
    addParameter(EffectParameter::makeFloat("vibrance", "Vibrance", -100.0, 100.0, 0.0));
    addParameter(EffectParameter::makeFloat("vignetteAmount", "Vignette Amount", -100.0, 100.0, 0.0));
}

std::unique_ptr<Effect> LumetriColorEffect::clone() const {
    auto e = std::make_unique<LumetriColorEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void LumetriColorEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    float temperature = getFloatParam("temperature") / 100.0f;
    float tint = getFloatParam("tint") / 100.0f;
    float exposure = getFloatParam("exposure");
    float contrast = 1.0f + getFloatParam("contrast") / 100.0f;
    float highlights = getFloatParam("highlights") / 200.0f;
    float shadows = getFloatParam("shadows") / 200.0f;
    float whites = getFloatParam("whites") / 200.0f;
    float blacks = getFloatParam("blacks") / 200.0f;
    float saturation = getFloatParam("saturation") / 100.0f;
    float vibrance = getFloatParam("vibrance") / 100.0f;
    float vignette = getFloatParam("vignetteAmount") / 100.0f;

    float expMul = std::pow(2.0f, exposure);
    float cx = buffer.width / 2.0f, cy = buffer.height / 2.0f;
    float maxDist = std::sqrt(cx * cx + cy * cy);

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            float r = p[0] / 255.0f, g = p[1] / 255.0f, b = p[2] / 255.0f;

            r += temperature * 0.1f;
            b -= temperature * 0.1f;
            g += tint * 0.05f;

            r *= expMul; g *= expMul; b *= expMul;

            float luma = 0.299f * r + 0.587f * g + 0.114f * b;
            r = luma + (r - luma) * contrast;
            g = luma + (g - luma) * contrast;
            b = luma + (b - luma) * contrast;

            if (luma > 0.5f) { float t = (luma - 0.5f) * 2.0f; r += highlights * t; g += highlights * t; b += highlights * t; }
            else { float t = (0.5f - luma) * 2.0f; r += shadows * t; g += shadows * t; b += shadows * t; }

            float peak = std::max({r, g, b});
            r += whites * (1.0f - peak);
            r -= blacks * (1.0f - luma);

            float cmax = std::max({r, g, b}), cmin = std::min({r, g, b});
            float sat2 = (cmax > 0.001f) ? (cmax - cmin) / cmax : 0;
            float vibAdj = 1.0f + vibrance * (1.0f - sat2);
            float satFinal = saturation * vibAdj;

            r = luma + (r - luma) * satFinal;
            g = luma + (g - luma) * satFinal;
            b = luma + (b - luma) * satFinal;

            if (std::abs(vignette) > 0.001f) {
                float dist = std::sqrt((x - cx) * (x - cx) + (y - cy) * (y - cy)) / maxDist;
                float vig = 1.0f - vignette * dist * dist;
                r *= vig; g *= vig; b *= vig;
            }

            p[0] = static_cast<uint8_t>(std::clamp(r * 255.0f, 0.0f, 255.0f));
            p[1] = static_cast<uint8_t>(std::clamp(g * 255.0f, 0.0f, 255.0f));
            p[2] = static_cast<uint8_t>(std::clamp(b * 255.0f, 0.0f, 255.0f));
        }
    }
}

} // namespace FreeEffect
