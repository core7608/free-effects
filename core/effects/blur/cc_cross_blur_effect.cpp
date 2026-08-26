#include "../effect_registry.h"
#include "cc_cross_blur_effect.h"
#include <algorithm>
#include <cmath>
#include <vector>

namespace FreeEffect {

static EffectRegistrar<CCCrossBlurEffect> s_reg("CC Cross Blur", "Blur & Sharpen");

CCCrossBlurEffect::CCCrossBlurEffect() {
    addParameter(EffectParameter::makeInt("radiusX", "Radius X", 1, 200, 10));
    addParameter(EffectParameter::makeInt("radiusY", "Radius Y", 1, 200, 10));
    addParameter(EffectParameter::makeDropdown("edgeBehavior", "Edge Behavior", {"Repeat", "Wrap"}, 0));
}

std::unique_ptr<Effect> CCCrossBlurEffect::clone() const {
    auto e = std::make_unique<CCCrossBlurEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void CCCrossBlurEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    int radiusX = getIntParam("radiusX");
    int radiusY = getIntParam("radiusY");
    if (radiusX <= 0 && radiusY <= 0) return;

    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);
    std::copy(buffer.data.begin(), buffer.data.end(), tmp.data.begin());

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            float rSum = 0, gSum = 0, bSum = 0, aSum = 0, wSum = 0;

            for (int dx = -radiusX; dx <= radiusX; dx++) {
                int sx = std::clamp(x + dx, 0, buffer.width - 1);
                const uint8_t* p = tmp.pixelAt(sx, y);
                float w = 1.0f - static_cast<float>(std::abs(dx)) / (radiusX + 1);
                rSum += p[0] * w; gSum += p[1] * w; bSum += p[2] * w; aSum += p[3] * w;
                wSum += w;
            }

            for (int dy = -radiusY; dy <= radiusY; dy++) {
                int sy = std::clamp(y + dy, 0, buffer.height - 1);
                const uint8_t* p = tmp.pixelAt(x, sy);
                float w = 1.0f - static_cast<float>(std::abs(dy)) / (radiusY + 1);
                rSum += p[0] * w; gSum += p[1] * w; bSum += p[2] * w; aSum += p[3] * w;
                wSum += w;
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
