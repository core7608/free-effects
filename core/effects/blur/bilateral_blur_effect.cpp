#include "../effect_registry.h"
#include "bilateral_blur_effect.h"
#include <algorithm>
#include <cmath>
#include <vector>

namespace FreeEffect {

static EffectRegistrar<BilateralBlurEffect> s_reg("Bilateral Blur", "Blur & Sharpen");

BilateralBlurEffect::BilateralBlurEffect() {
    addParameter(EffectParameter::makeInt("radius", "Blur Radius", 1, 50, 10));
    addParameter(EffectParameter::makeFloat("thresholdLuma", "Threshold Luma", 1.0, 255.0, 25.0));
    addParameter(EffectParameter::makeFloat("thresholdChroma", "Threshold Chroma", 1.0, 255.0, 25.0));
}

std::unique_ptr<Effect> BilateralBlurEffect::clone() const {
    auto e = std::make_unique<BilateralBlurEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void BilateralBlurEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    int radius = getIntParam("radius");
    float threshLuma = getFloatParam("thresholdLuma");
    float threshChroma = getFloatParam("thresholdChroma");
    if (radius <= 0) return;

    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);
    std::copy(buffer.data.begin(), buffer.data.end(), tmp.data.begin());

    float sigmaSpace = static_cast<float>(radius) / 3.0f;

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            const uint8_t* center = tmp.pixelAt(x, y);
            float cLuma = 0.299f * center[0] + 0.587f * center[1] + 0.114f * center[2];

            float rSum = 0, gSum = 0, bSum = 0, aSum = 0, wSum = 0;

            for (int dy = -radius; dy <= radius; dy++) {
                for (int dx = -radius; dx <= radius; dx++) {
                    int sx = std::clamp(x + dx, 0, buffer.width - 1);
                    int sy = std::clamp(y + dy, 0, buffer.height - 1);
                    const uint8_t* p = tmp.pixelAt(sx, sy);

                    float pLuma = 0.299f * p[0] + 0.587f * p[1] + 0.114f * p[2];
                    float dLuma = std::abs(pLuma - cLuma);
                    float dChroma = std::sqrt(
                        std::pow(p[0] - center[0], 2) +
                        std::pow(p[1] - center[1], 2) +
                        std::pow(p[2] - center[2], 2));

                    float wSpace = std::exp(-(dx * dx + dy * dy) / (2.0f * sigmaSpace * sigmaSpace));
                    float wRangeL = std::exp(-(dLuma * dLuma) / (2.0f * threshLuma * threshLuma));
                    float wRangeC = std::exp(-(dChroma * dChroma) / (2.0f * threshChroma * threshChroma));
                    float w = wSpace * wRangeL * wRangeC;

                    rSum += p[0] * w;
                    gSum += p[1] * w;
                    bSum += p[2] * w;
                    aSum += p[3] * w;
                    wSum += w;
                }
            }

            uint8_t* dst = buffer.pixelAt(x, y);
            if (wSum > 0) {
                dst[0] = static_cast<uint8_t>(std::clamp(static_cast<double>(rSum / wSum), 0.0, 255.0));
                dst[1] = static_cast<uint8_t>(std::clamp(static_cast<double>(gSum / wSum), 0.0, 255.0));
                dst[2] = static_cast<uint8_t>(std::clamp(static_cast<double>(bSum / wSum), 0.0, 255.0));
                dst[3] = static_cast<uint8_t>(std::clamp(static_cast<double>(aSum / wSum), 0.0, 255.0));
            }
        }
    }
}

} // namespace FreeEffect
