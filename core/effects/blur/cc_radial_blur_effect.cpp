#include "../effect_registry.h"
#include "cc_radial_blur_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<CCRadialBlurEffect> s_reg("CC Radial Blur", "Blur & Sharpen");

CCRadialBlurEffect::CCRadialBlurEffect() {
    addParameter(EffectParameter::makeFloat("amount", "Amount", 0.0, 360.0, 10.0));
    addParameter(EffectParameter::makeVec2("center", "Center", {0.5, 0.5}));
    addParameter(EffectParameter::makeDropdown("type", "Type", {"Spin", "Zoom"}, 0));
    addParameter(EffectParameter::makeInt("quality", "Quality", 1, 10, 5));
}

std::unique_ptr<Effect> CCRadialBlurEffect::clone() const {
    auto e = std::make_unique<CCRadialBlurEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void CCRadialBlurEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    float amount = getFloatParam("amount");
    Vec2 center = getVec2Param("center");
    bool isSpin = (getDropdownParam("type") == 0);
    int quality = getIntParam("quality");

    float cx = center.x * buffer.width;
    float cy = center.y * buffer.height;
    int steps = std::max(quality, 1);

    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);
    std::copy(buffer.data.begin(), buffer.data.end(), tmp.data.begin());

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            float rSum = 0, gSum = 0, bSum = 0, aSum = 0;
            float wSum = 0;

            for (int s = 0; s < steps; s++) {
                float t = (static_cast<float>(s) / steps - 0.5f) * amount;
                float sx, sy;

                if (isSpin) {
                    float rad = t * 3.14159265f / 180.0f;
                    float dx = x - cx, dy = y - cy;
                    sx = cx + dx * std::cos(rad) - dy * std::sin(rad);
                    sy = cy + dx * std::sin(rad) + dy * std::cos(rad);
                } else {
                    float scale = 1.0f + t / 100.0f;
                    sx = cx + (x - cx) * scale;
                    sy = cy + (y - cy) * scale;
                }

                int px = std::clamp(static_cast<int>(sx + 0.5f), 0, buffer.width - 1);
                int py = std::clamp(static_cast<int>(sy + 0.5f), 0, buffer.height - 1);
                const uint8_t* p = tmp.pixelAt(px, py);
                float w = 1.0f;
                rSum += p[0] * w; gSum += p[1] * w; bSum += p[2] * w; aSum += p[3] * w;
                wSum += w;
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
