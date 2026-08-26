#include "../effect_registry.h"
#include "cc_vector_blur_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<CCVectorBlurEffect> s_reg("CC Vector Blur", "Blur & Sharpen");

CCVectorBlurEffect::CCVectorBlurEffect() {
    addParameter(EffectParameter::makeFloat("amount", "Amount", -100.0, 100.0, 20.0));
    addParameter(EffectParameter::makeAngle("angle", "Angle", 0.0));
    addParameter(EffectParameter::makeInt("radius", "Softness", 1, 100, 10));
}

std::unique_ptr<Effect> CCVectorBlurEffect::clone() const {
    auto e = std::make_unique<CCVectorBlurEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void CCVectorBlurEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    float amount = getFloatParam("amount");
    float angle = getFloatParam("angle") * 3.14159265f / 180.0f;
    int radius = getIntParam("radius");
    if (radius <= 0 || std::abs(amount) < 0.01f) return;

    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);
    std::copy(buffer.data.begin(), buffer.data.end(), tmp.data.begin());

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            const uint8_t* px = tmp.pixelAt(x, y);
            float luma = (0.299f * px[0] + 0.587f * px[1] + 0.114f * px[2]) / 255.0f;
            float dx = std::cos(angle) * amount * luma;
            float dy = std::sin(angle) * amount * luma;

            float rSum = 0, gSum = 0, bSum = 0, aSum = 0, wSum = 0;
            for (int s = -radius; s <= radius; s++) {
                float t = static_cast<float>(s) / radius;
                float offX = dx * t;
                float offY = dy * t;
                int sx = std::clamp(static_cast<int>(x + offX + 0.5f), 0, buffer.width - 1);
                int sy = std::clamp(static_cast<int>(y + offY + 0.5f), 0, buffer.height - 1);
                const uint8_t* p = tmp.pixelAt(sx, sy);
                float w = 1.0f - std::abs(t);
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
