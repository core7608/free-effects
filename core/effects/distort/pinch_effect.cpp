#include "../effect_registry.h"
#include "pinch_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<PinchEffect> s_reg("Pinch", "Distort");

PinchEffect::PinchEffect() {
    addParameter(EffectParameter::makeFloat("amount", "Amount", -1.0, 1.0, 0.5));
}

std::vector<ParameterGroup> PinchEffect::getParameterGroups() const {
    return {{getName(), {EffectParameter::makeFloat("amount", "Amount", -1.0, 1.0, false)}}};
}

std::unique_ptr<Effect> PinchEffect::clone() const {
    auto e = std::make_unique<PinchEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void PinchEffect::render(PixelBuffer& buffer, double time) {
    float amount = getFloatParam("amount");
    float cx = buffer.width / 2.0f;
    float cy = buffer.height / 2.0f;
    float maxR = std::max(buffer.width, buffer.height) / 2.0f;

    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            float dx = (x - cx) / maxR;
            float dy = (y - cy) / maxR;
            float dist = std::sqrt(dx * dx + dy * dy);
            float factor = 1.0f;
            if (dist < 1.0f) {
                factor = 1.0f - amount * (1.0f - dist) * (1.0f - dist);
            }
            int sx = static_cast<int>(cx + (x - cx) * factor);
            int sy = static_cast<int>(cy + (y - cy) * factor);
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
