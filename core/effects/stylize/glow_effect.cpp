#include "../effect_registry.h"
#include "glow_effect.h"
#include <algorithm>
#include <cmath>
#include <vector>

namespace FreeEffect {

static EffectRegistrar<GlowEffect> s_reg("Glow", "Stylize");

GlowEffect::GlowEffect() {
    addParameter(EffectParameter::makeFloat("threshold", "Glow Threshold", 0.0, 100.0, 60.0));
    addParameter(EffectParameter::makeFloat("radius", "Glow Radius", 0.0, 200.0, 10.0));
    addParameter(EffectParameter::makeFloat("intensity", "Glow Intensity", 0.0, 10.0, 1.0));
}

std::vector<ParameterGroup> GlowEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeFloat("threshold", "Glow Threshold", 0.0, 100.0, false),
        EffectParameter::makeFloat("radius", "Glow Radius", 0.0, 200.0, false),
        EffectParameter::makeFloat("intensity", "Glow Intensity", 0.0, 10.0, false)
    }}};
}

std::unique_ptr<Effect> GlowEffect::clone() const {
    auto e = std::make_unique<GlowEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void GlowEffect::render(PixelBuffer& buffer, double time) {
    float threshold = getFloatParam("threshold") / 100.0f * 255.0f;
    int radius = static_cast<int>(getFloatParam("radius"));
    float intensity = getFloatParam("intensity");

    PixelBuffer bright;
    bright.resize(buffer.width, buffer.height);
    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            const uint8_t* p = buffer.pixelAt(x, y);
            uint8_t* dst = bright.pixelAt(x, y);
            float lum = p[0] * 0.299f + p[1] * 0.587f + p[2] * 0.114f;
            if (lum >= threshold) {
                dst[0] = p[0]; dst[1] = p[1]; dst[2] = p[2]; dst[3] = p[3];
            }
        }
    }

    if (radius > 0) {
        int ksize = radius * 2 + 1;
        std::vector<float> kernel(ksize);
        float sigma = radius / 3.0f;
        float sum = 0;
        for (int i = 0; i < ksize; i++) {
            float x = static_cast<float>(i - radius);
            kernel[i] = std::exp(-(x * x) / (2.0f * sigma * sigma));
            sum += kernel[i];
        }
        for (auto& k : kernel) k /= sum;

        PixelBuffer tmp;
        tmp.resize(buffer.width, buffer.height);
        for (int y = 0; y < buffer.height; y++) {
            for (int x = 0; x < buffer.width; x++) {
                float r = 0, g = 0, b = 0;
                for (int k = 0; k < ksize; k++) {
                    int sx = std::clamp(x + k - radius, 0, buffer.width - 1);
                    const uint8_t* p = bright.pixelAt(sx, y);
                    float w = kernel[k];
                    r += p[0] * w; g += p[1] * w; b += p[2] * w;
                }
                uint8_t* dst = tmp.pixelAt(x, y);
                dst[0] = static_cast<uint8_t>(r); dst[1] = static_cast<uint8_t>(g);
                dst[2] = static_cast<uint8_t>(b); dst[3] = bright.pixelAt(x, y)[3];
            }
        }
        bright.data = tmp.data;
    }

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            const uint8_t* g2 = bright.pixelAt(x, y);
            for (int c = 0; c < 3; c++) {
                float v = p[c] + g2[c] * intensity;
                p[c] = static_cast<uint8_t>(std::clamp(static_cast<double>(v), 0.0, 255.0));
            }
        }
    }
}

} // namespace FreeEffect
