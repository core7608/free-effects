#include "../effect_registry.h"
#include "depth_of_field_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<DepthOfFieldEffect> s_reg("Depth of Field", "Blur & Sharpen");

DepthOfFieldEffect::DepthOfFieldEffect() {
    addParameter(EffectParameter::makeFloat("focalDistance", "Focal Distance", 0.0, 1000.0, 500.0));
    addParameter(EffectParameter::makeFloat("focalRange", "Focal Range", 0.0, 500.0, 100.0));
    addParameter(EffectParameter::makeInt("maxBlur", "Maximum Blur", 1, 100, 15));
    addParameter(EffectParameter::makeFloat("depthCenter", "Depth Center", 0.0, 1.0, 0.5));
}

std::unique_ptr<Effect> DepthOfFieldEffect::clone() const {
    auto e = std::make_unique<DepthOfFieldEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void DepthOfFieldEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    float focalDist = getFloatParam("focalDistance");
    float focalRange = getFloatParam("focalRange");
    int maxBlur = getIntParam("maxBlur");
    float depthCenter = getFloatParam("depthCenter");
    if (maxBlur <= 0) return;

    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);
    std::copy(buffer.data.begin(), buffer.data.end(), tmp.data.begin());

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* px = tmp.pixelAt(x, y);
            float depthSim = static_cast<float>(y) / buffer.height;
            float dist = std::abs(depthSim - depthCenter);
            float blurAmount = std::max(0.0f, dist - focalRange / 2000.0f);
            int radius = static_cast<int>(blurAmount * maxBlur * 2.0f);
            radius = std::min(radius, maxBlur);

            if (radius <= 0) continue;

            float rSum = 0, gSum = 0, bSum = 0, aSum = 0, wSum = 0;
            for (int dy = -radius; dy <= radius; dy++) {
                for (int dx = -radius; dx <= radius; dx++) {
                    int sx = std::clamp(x + dx, 0, buffer.width - 1);
                    int sy = std::clamp(y + dy, 0, buffer.height - 1);
                    const uint8_t* p = tmp.pixelAt(sx, sy);
                    float d = std::sqrt(static_cast<float>(dx * dx + dy * dy));
                    float w = (d <= radius) ? 1.0f - d / radius : 0.0f;
                    rSum += p[0] * w; gSum += p[1] * w; bSum += p[2] * w; aSum += p[3] * w;
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
