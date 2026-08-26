#include "../effect_registry.h"
#include "unsharp_mask_effect.h"
#include <algorithm>
#include <cmath>
#include <vector>

namespace FreeEffect {

static EffectRegistrar<UnsharpMaskEffect> s_reg("Unsharp Mask", "Blur & Sharpen");

UnsharpMaskEffect::UnsharpMaskEffect() {
    addParameter(EffectParameter::makeFloat("amount", "Amount", 0.0, 200.0, 50.0));
    addParameter(EffectParameter::makeFloat("radius", "Radius", 0.0, 100.0, 5.0));
    addParameter(EffectParameter::makeFloat("threshold", "Threshold", 0.0, 255.0, 0.0));
}

std::vector<ParameterGroup> UnsharpMaskEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeFloat("amount", "Amount", 0.0, 200.0, false),
        EffectParameter::makeFloat("radius", "Radius", 0.0, 100.0, false),
        EffectParameter::makeFloat("threshold", "Threshold", 0.0, 255.0, false)
    }}};
}

std::unique_ptr<Effect> UnsharpMaskEffect::clone() const {
    auto e = std::make_unique<UnsharpMaskEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void UnsharpMaskEffect::render(PixelBuffer& buffer, double time) {
    float amount = getFloatParam("amount") / 100.0f;
    int radius = static_cast<int>(getFloatParam("radius"));
    float threshold = getFloatParam("threshold");
    if (radius <= 0 || amount <= 0) return;

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

    PixelBuffer blurred;
    blurred.resize(buffer.width, buffer.height);

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            float r = 0, g = 0, b = 0;
            for (int k = 0; k < ksize; k++) {
                int sx = std::clamp(x + k - radius, 0, buffer.width - 1);
                const uint8_t* p = buffer.pixelAt(sx, y);
                float w = kernel[k];
                r += p[0] * w; g += p[1] * w; b += p[2] * w;
            }
            uint8_t* dst = blurred.pixelAt(x, y);
            dst[0] = static_cast<uint8_t>(r);
            dst[1] = static_cast<uint8_t>(g);
            dst[2] = static_cast<uint8_t>(b);
            dst[3] = buffer.pixelAt(x, y)[3];
        }
    }

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            float r = 0, g = 0, b = 0;
            for (int k = 0; k < ksize; k++) {
                int sy = std::clamp(y + k - radius, 0, buffer.height - 1);
                const uint8_t* p = blurred.pixelAt(x, sy);
                float w = kernel[k];
                r += p[0] * w; g += p[1] * w; b += p[2] * w;
            }
            uint8_t* bd = blurred.pixelAt(x, y);
            bd[0] = static_cast<uint8_t>(r);
            bd[1] = static_cast<uint8_t>(g);
            bd[2] = static_cast<uint8_t>(b);
        }
    }

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            const uint8_t* orig = buffer.pixelAt(x, y);
            const uint8_t* blur = blurred.pixelAt(x, y);
            uint8_t* dst = buffer.pixelAt(x, y);
            for (int c = 0; c < 3; c++) {
                float diff = static_cast<float>(orig[c]) - static_cast<float>(blur[c]);
                if (std::abs(diff) >= threshold) {
                    float val = static_cast<float>(orig[c]) + diff * amount;
                    dst[c] = static_cast<uint8_t>(std::clamp(static_cast<double>(val), 0.0, 255.0));
                }
            }
        }
    }
}

} // namespace FreeEffect
