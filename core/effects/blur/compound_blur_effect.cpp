#include "../effect_registry.h"
#include "compound_blur_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<CompoundBlurEffect> s_reg("Compound Blur", "Blur & Sharpen");

CompoundBlurEffect::CompoundBlurEffect() {
    addParameter(EffectParameter::makeInt("maximumRadius", "Maximum Blur Radius", 1, 100, 20));
    addParameter(EffectParameter::makeBool("invertBlur", "Invert Blur Map", false));
}

std::unique_ptr<Effect> CompoundBlurEffect::clone() const {
    auto e = std::make_unique<CompoundBlurEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void CompoundBlurEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    int maxRadius = getIntParam("maximumRadius");
    bool invert = getBoolParam("invertBlur");
    if (maxRadius <= 0) return;

    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);
    std::copy(buffer.data.begin(), buffer.data.end(), tmp.data.begin());

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            const uint8_t* px = tmp.pixelAt(x, y);
            float luma = (0.299f * px[0] + 0.587f * px[1] + 0.114f * px[2]) / 255.0f;
            if (invert) luma = 1.0f - luma;
            int radius = static_cast<int>(luma * maxRadius + 0.5f);
            if (radius <= 0) {
                uint8_t* dst = buffer.pixelAt(x, y);
                dst[0] = px[0]; dst[1] = px[1]; dst[2] = px[2]; dst[3] = px[3];
                continue;
            }

            float rSum = 0, gSum = 0, bSum = 0, aSum = 0, wSum = 0;
            for (int dy = -radius; dy <= radius; dy++) {
                for (int dx = -radius; dx <= radius; dx++) {
                    int sx = std::clamp(x + dx, 0, buffer.width - 1);
                    int sy = std::clamp(y + dy, 0, buffer.height - 1);
                    const uint8_t* p = tmp.pixelAt(sx, sy);
                    float w = 1.0f;
                    rSum += p[0] * w; gSum += p[1] * w; bSum += p[2] * w; aSum += p[3] * w;
                    wSum += w;
                }
            }

            uint8_t* dst = buffer.pixelAt(x, y);
            dst[0] = static_cast<uint8_t>(std::clamp(static_cast<double>(rSum / wSum), 0.0, 255.0));
            dst[1] = static_cast<uint8_t>(std::clamp(static_cast<double>(gSum / wSum), 0.0, 255.0));
            dst[2] = static_cast<uint8_t>(std::clamp(static_cast<double>(bSum / wSum), 0.0, 255.0));
            dst[3] = static_cast<uint8_t>(std::clamp(static_cast<double>(aSum / wSum), 0.0, 255.0));
        }
    }
}

} // namespace FreeEffect
