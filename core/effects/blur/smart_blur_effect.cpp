#include "../effect_registry.h"
#include "smart_blur_effect.h"
#include <algorithm>
#include <cmath>
#include <vector>

namespace FreeEffect {

static EffectRegistrar<SmartBlurEffect> s_reg("Smart Blur", "Blur & Sharpen");

SmartBlurEffect::SmartBlurEffect() {
    addParameter(EffectParameter::makeFloat("blurRadius", "Blur Radius", 1.0, 100.0, 10.0));
    addParameter(EffectParameter::makeFloat("threshold", "Threshold", 0.0, 255.0, 25.0));
    addParameter(EffectParameter::makeInt("quality", "Quality", 1, 10, 3));
}

std::unique_ptr<Effect> SmartBlurEffect::clone() const {
    auto e = std::make_unique<SmartBlurEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void SmartBlurEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    int radius = static_cast<int>(getFloatParam("blurRadius"));
    float threshold = getFloatParam("threshold");
    int quality = getIntParam("quality");
    if (radius <= 0) return;

    for (int q = 0; q < quality; q++) {
        PixelBuffer tmp;
        tmp.resize(buffer.width, buffer.height);
        std::copy(buffer.data.begin(), buffer.data.end(), tmp.data.begin());

        for (int y = 0; y < buffer.height; y++) {
            for (int x = 0; x < buffer.width; x++) {
                const uint8_t* center = tmp.pixelAt(x, y);
                float rSum = 0, gSum = 0, bSum = 0, aSum = 0;
                float wSum = 0;

                for (int dy = -radius; dy <= radius; dy++) {
                    for (int dx = -radius; dx <= radius; dx++) {
                        int sx = std::clamp(x + dx, 0, buffer.width - 1);
                        int sy = std::clamp(y + dy, 0, buffer.height - 1);
                        const uint8_t* p = tmp.pixelAt(sx, sy);

                        float dr = p[0] - center[0];
                        float dg = p[1] - center[1];
                        float db = p[2] - center[2];
                        float diff = std::sqrt(dr * dr + dg * dg + db * db);

                        float w = (diff < threshold) ? 1.0f - (diff / threshold) : 0.0f;
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
}

} // namespace FreeEffect
