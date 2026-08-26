#include "../effect_registry.h"
#include "twirl_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<TwirlEffect> s_reg("Twirl", "Distort");

TwirlEffect::TwirlEffect() {
    addParameter(EffectParameter::makeAngle("angle", "Twirl Angle", 180.0));
    addParameter(EffectParameter::makeFloat("radius", "Radius", 0.0, 100.0, 50.0));
}

std::vector<ParameterGroup> TwirlEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeAngle("angle", "Twirl Angle", 180.0),
        EffectParameter::makeFloat("radius", "Radius", 0.0, 100.0, false)
    }}};
}

std::unique_ptr<Effect> TwirlEffect::clone() const {
    auto e = std::make_unique<TwirlEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void TwirlEffect::render(PixelBuffer& buffer, double time) {
    float angle = getAngleParam("angle") * 3.14159265f / 180.0f;
    float rad = getFloatParam("radius") / 100.0f;
    float cx = buffer.width / 2.0f;
    float cy = buffer.height / 2.0f;
    float maxR = rad * std::max(buffer.width, buffer.height);

    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            float dx = x - cx;
            float dy = y - cy;
            float dist = std::sqrt(dx * dx + dy * dy);
            float origAngle = std::atan2(dy, dx);
            float twirlAngle = 0.0f;
            if (dist < maxR && maxR > 0) {
                twirlAngle = angle * (1.0f - dist / maxR);
            }
            float newAngle = origAngle + twirlAngle;
            int sx = static_cast<int>(cx + dist * std::cos(newAngle));
            int sy = static_cast<int>(cy + dist * std::sin(newAngle));
            sx = std::clamp(sx, 0, buffer.width - 1);
            sy = std::clamp(sy, 0, buffer.height - 1);
            const uint8_t* src = buffer.pixelAt(sx, sy);
            uint8_t* dst = tmp.pixelAt(x, y);
            dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3];
        }
    }
    buffer.data = tmp.data;
}

} // namespace FreeEffect
