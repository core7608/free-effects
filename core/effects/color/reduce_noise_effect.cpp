#include "../effect_registry.h"
#include "reduce_noise_effect.h"
#include <algorithm>
#include <cmath>
#include <vector>

namespace FreeEffect {

static EffectRegistrar<ReduceNoiseEffect> s_reg("Reduce Noise", "Color Correction");

ReduceNoiseEffect::ReduceNoiseEffect() {
    addParameter(EffectParameter::makeInt("spatialRadius", "Spatial Radius", 1, 10, 3));
    addParameter(EffectParameter::makeFloat("threshold", "Threshold", 0.0, 255.0, 20.0));
    addParameter(EffectParameter::makeFloat("strength", "Strength", 0.0, 100.0, 50.0));
}

std::unique_ptr<Effect> ReduceNoiseEffect::clone() const {
    auto e = std::make_unique<ReduceNoiseEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void ReduceNoiseEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    int radius = getIntParam("spatialRadius");
    float threshold = getFloatParam("threshold");
    float strength = getFloatParam("strength") / 100.0f;
    if (radius <= 0) return;

    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);
    std::copy(buffer.data.begin(), buffer.data.end(), tmp.data.begin());

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            const uint8_t* center = tmp.pixelAt(x, y);
            float rSum = 0, gSum = 0, bSum = 0, wSum = 0;

            for (int dy = -radius; dy <= radius; dy++) {
                for (int dx = -radius; dx <= radius; dx++) {
                    int sx = std::clamp(x + dx, 0, buffer.width - 1);
                    int sy = std::clamp(y + dy, 0, buffer.height - 1);
                    const uint8_t* p = tmp.pixelAt(sx, sy);
                    float diff = std::abs(p[0] - center[0]) + std::abs(p[1] - center[1]) + std::abs(p[2] - center[2]);
                    float w = (diff < threshold) ? 1.0f : std::exp(-(diff - threshold) / 100.0f);
                    rSum += p[0] * w; gSum += p[1] * w; bSum += p[2] * w;
                    wSum += w;
                }
            }

            uint8_t* dst = buffer.pixelAt(x, y);
            if (wSum > 0) {
                float nr = rSum / wSum, ng = gSum / wSum, nb = bSum / wSum;
                dst[0] = static_cast<uint8_t>(std::clamp(center[0] + (nr - center[0]) * strength, 0.0f, 255.0f));
                dst[1] = static_cast<uint8_t>(std::clamp(center[1] + (ng - center[1]) * strength, 0.0f, 255.0f));
                dst[2] = static_cast<uint8_t>(std::clamp(center[2] + (nb - center[2]) * strength, 0.0f, 255.0f));
                dst[3] = center[3];
            }
        }
    }
}

} // namespace FreeEffect
