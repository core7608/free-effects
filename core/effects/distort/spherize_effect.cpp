#include "../effect_registry.h"
#include "spherize_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<SpherizeEffect> s_reg("Spherize", "Distort");

SpherizeEffect::SpherizeEffect() {
    addParameter(EffectParameter::makeFloat("amount", "Amount", -1.0, 1.0, 0.5));
    addParameter(EffectParameter::makeFloat("radius", "Radius", 0.0, 100.0, 50.0));
}

std::vector<ParameterGroup> SpherizeEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeFloat("amount", "Amount", -1.0, 1.0, false),
        EffectParameter::makeFloat("radius", "Radius", 0.0, 100.0, false)
    }}};
}

std::unique_ptr<Effect> SpherizeEffect::clone() const {
    auto e = std::make_unique<SpherizeEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void SpherizeEffect::render(PixelBuffer& buffer, double time) {
    float amount = getFloatParam("amount");
    float rad = getFloatParam("radius") / 100.0f;
    float cx = buffer.width / 2.0f;
    float cy = buffer.height / 2.0f;
    float maxR = rad * std::max(buffer.width, buffer.height);

    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            float dx = (x - cx) / maxR;
            float dy = (y - cy) / maxR;
            float dist = std::sqrt(dx * dx + dy * dy);
            float factor = 1.0f;
            if (dist < 1.0f && maxR > 0) {
                factor = 1.0f + (1.0f - dist * dist) * amount;
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
