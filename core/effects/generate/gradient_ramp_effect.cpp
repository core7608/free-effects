#include "../effect_registry.h"
#include "gradient_ramp_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<GradientRampEffect> s_reg("Gradient Ramp", "Generate");

GradientRampEffect::GradientRampEffect() {
    addParameter(EffectParameter::makeColor("startColor", "Start Color", {0.0, 0.0, 0.0, 1.0}));
    addParameter(EffectParameter::makeColor("endColor", "End Color", {255.0, 255.0, 255.0, 1.0}));
    addParameter(EffectParameter::makeVec2("startPoint", "Start Point", {0.0, 0.0}));
    addParameter(EffectParameter::makeVec2("endPoint", "End Point", {1.0, 1.0}));
}

std::vector<ParameterGroup> GradientRampEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeColor("startColor", "Start Color", {0.0, 0.0, 0.0, 1.0}),
        EffectParameter::makeColor("endColor", "End Color", {255.0, 255.0, 255.0, 1.0}),
        EffectParameter::makeVec2("startPoint", "Start Point", {0.0, 0.0}),
        EffectParameter::makeVec2("endPoint", "End Point", {1.0, 1.0})
    }}};
}

std::unique_ptr<Effect> GradientRampEffect::clone() const {
    auto e = std::make_unique<GradientRampEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void GradientRampEffect::render(PixelBuffer& buffer, double time) {
    Color sc = getColorParam("startColor");
    Color ec = getColorParam("endColor");
    Vec2 sp = getVec2Param("startPoint");
    Vec2 ep = getVec2Param("endPoint");

    float sx = sp.x * buffer.width;
    float sy = sp.y * buffer.height;
    float ex = ep.x * buffer.width;
    float ey = ep.y * buffer.height;
    float dx = ex - sx;
    float dy = ey - sy;
    float len2 = dx * dx + dy * dy;

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            float px = x - sx;
            float py = y - sy;
            float t = (len2 > 0.001f) ? std::clamp((px * dx + py * dy) / len2, 0.0f, 1.0f) : 0.0f;
            uint8_t* p = buffer.pixelAt(x, y);
            p[0] = static_cast<uint8_t>(sc.r + (ec.r - sc.r) * t);
            p[1] = static_cast<uint8_t>(sc.g + (ec.g - sc.g) * t);
            p[2] = static_cast<uint8_t>(sc.b + (ec.b - sc.b) * t);
            p[3] = static_cast<uint8_t>(sc.a * 255.0 + (ec.a - sc.a) * 255.0 * t);
        }
    }
}

} // namespace FreeEffect
