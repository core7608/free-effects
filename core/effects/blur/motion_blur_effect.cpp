#include "../effect_registry.h"
#include "motion_blur_effect.h"
#include <cmath>
#include <vector>

namespace FreeEffect {

static EffectRegistrar<MotionBlurEffect> s_reg("Motion Blur", "Blur & Sharpen");

MotionBlurEffect::MotionBlurEffect() {
    addParameter(EffectParameter::makeAngle("angle", "Direction", 0.0));
    addParameter(EffectParameter::makeFloat("amount", "Amount", 0.0, 100.0, 10.0));
}

std::vector<ParameterGroup> MotionBlurEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeAngle("angle", "Direction", 0.0),
        EffectParameter::makeFloat("amount", "Amount", 0.0, 100.0, false)
    }}};
}

std::unique_ptr<Effect> MotionBlurEffect::clone() const {
    auto e = std::make_unique<MotionBlurEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void MotionBlurEffect::render(PixelBuffer& buffer, double time) {
    float amount = getFloatParam("amount");
    float angle = getAngleParam("angle") * 3.14159265f / 180.0f;
    int samples = static_cast<int>(amount) + 1;
    if (samples <= 1) return;

    float dx = std::cos(angle);
    float dy = std::sin(angle);

    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            float r = 0, g = 0, b = 0, a = 0;
            for (int s = 0; s < samples; s++) {
                float t = (static_cast<float>(s) / (samples - 1) - 0.5f) * amount;
                int sx = static_cast<int>(x + dx * t);
                int sy = static_cast<int>(y + dy * t);
                sx = std::clamp(sx, 0, buffer.width - 1);
                sy = std::clamp(sy, 0, buffer.height - 1);
                const uint8_t* p = buffer.pixelAt(sx, sy);
                r += p[0]; g += p[1]; b += p[2]; a += p[3];
            }
            uint8_t* dst = tmp.pixelAt(x, y);
            dst[0] = static_cast<uint8_t>(r / samples);
            dst[1] = static_cast<uint8_t>(g / samples);
            dst[2] = static_cast<uint8_t>(b / samples);
            dst[3] = static_cast<uint8_t>(a / samples);
        }
    }
    buffer.data = tmp.data;
}

} // namespace FreeEffect
