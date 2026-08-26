#include "../effect_registry.h"
#include "gradient_map_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<GradientMapEffect> s_reg("Gradient Map", "Color Correction");

GradientMapEffect::GradientMapEffect() {
    addParameter(EffectParameter::makeColor("color0", "Color 1", {0.0, 0.0, 0.0, 1.0}));
    addParameter(EffectParameter::makeColor("color1", "Color 2", {255.0, 255.0, 255.0, 1.0}));
    addParameter(EffectParameter::makeColor("color2", "Color 3", {255.0, 0.0, 0.0, 1.0}));
    addParameter(EffectParameter::makeBool("useColor3", "Use Color 3", false));
    addParameter(EffectParameter::makeFloat("blendWithOriginal", "Blend With Original", 0.0, 100.0, 0.0));
}

std::unique_ptr<Effect> GradientMapEffect::clone() const {
    auto e = std::make_unique<GradientMapEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void GradientMapEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    Color c0 = getColorParam("color0");
    Color c1 = getColorParam("color1");
    Color c2 = getColorParam("color2");
    bool useC2 = getBoolParam("useColor3");
    float blend = getFloatParam("blendWithOriginal") / 100.0f;

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            float luma = (0.299f * p[0] + 0.587f * p[1] + 0.114f * p[2]) / 255.0f;

            float r, g, b;
            if (!useC2) {
                r = c0.r + (c1.r - c0.r) * luma;
                g = c0.g + (c1.g - c0.g) * luma;
                b = c0.b + (c1.b - c0.b) * luma;
            } else {
                if (luma < 0.5f) {
                    float t = luma * 2.0f;
                    r = c0.r + (c1.r - c0.r) * t;
                    g = c0.g + (c1.g - c0.g) * t;
                    b = c0.b + (c1.b - c0.b) * t;
                } else {
                    float t = (luma - 0.5f) * 2.0f;
                    r = c1.r + (c2.r - c1.r) * t;
                    g = c1.g + (c2.g - c1.g) * t;
                    b = c1.b + (c2.b - c1.b) * t;
                }
            }

            p[0] = static_cast<uint8_t>(std::clamp(static_cast<double>(p[0] * blend + r * (1.0f - blend)), 0.0, 255.0));
            p[1] = static_cast<uint8_t>(std::clamp(static_cast<double>(p[1] * blend + g * (1.0f - blend)), 0.0, 255.0));
            p[2] = static_cast<uint8_t>(std::clamp(static_cast<double>(p[2] * blend + b * (1.0f - blend)), 0.0, 255.0));
        }
    }
}

} // namespace FreeEffect
