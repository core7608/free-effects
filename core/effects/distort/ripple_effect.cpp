#include "../effect_registry.h"
#include "ripple_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<RippleEffect> s_reg("Ripple", "Distort");

RippleEffect::RippleEffect() {
    addParameter(EffectParameter::makeVec2("center", "Center", {0.5, 0.5}));
    addParameter(EffectParameter::makeFloat("amount", "Amount", -100.0, 100.0, 10.0));
    addParameter(EffectParameter::makeFloat("width", "Ripple Width", 1.0, 200.0, 20.0));
    addParameter(EffectParameter::makeFloat("speed", "Speed", -10.0, 10.0, 1.0));
}

std::vector<ParameterGroup> RippleEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeVec2("center", "Center", {0.5, 0.5}),
        EffectParameter::makeFloat("amount", "Amount", -100.0, 100.0, false),
        EffectParameter::makeFloat("width", "Ripple Width", 1.0, 200.0, false),
        EffectParameter::makeFloat("speed", "Speed", -10.0, 10.0, false)
    }}};
}

std::unique_ptr<Effect> RippleEffect::clone() const {
    auto e = std::make_unique<RippleEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void RippleEffect::render(PixelBuffer& buffer, double time) {
    Vec2 center = getVec2Param("center");
    float amount = getFloatParam("amount");
    float width = getFloatParam("width");
    float speed = getFloatParam("speed");
    float cx = center.x * buffer.width;
    float cy = center.y * buffer.height;

    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            float dx = x - cx;
            float dy = y - cy;
            float dist = std::sqrt(dx * dx + dy * dy);
            float wave = std::sin(dist / std::max(width, 1.0f) - time * speed) * amount;
            float nx = (dist > 0.001f) ? dx / dist : 0.0f;
            float ny = (dist > 0.001f) ? dy / dist : 0.0f;
            int sx = static_cast<int>(x + nx * wave);
            int sy = static_cast<int>(y + ny * wave);
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
