#include "../effect_registry.h"
#include "circle_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<CircleEffect> s_reg("Circle", "Generate");

CircleEffect::CircleEffect() {
    addParameter(EffectParameter::makeVec2("center", "Center", {0.5, 0.5}));
    addParameter(EffectParameter::makeFloat("radius", "Radius", 0.0, 1000.0, 50.0));
    addParameter(EffectParameter::makeFloat("feather", "Edge Feather", 0.0, 100.0, 0.0));
    addParameter(EffectParameter::makeColor("color", "Color", {255.0, 255.0, 255.0, 1.0}));
    addParameter(EffectParameter::makeBool("invert", "Invert", false));
}

std::unique_ptr<Effect> CircleEffect::clone() const {
    auto e = std::make_unique<CircleEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void CircleEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    Vec2 center = getVec2Param("center");
    float radius = getFloatParam("radius");
    float feather = getFloatParam("feather") / 100.0f;
    Color col = getColorParam("color");
    bool invert = getBoolParam("invert");

    float cx = center.x * buffer.width, cy = center.y * buffer.height;
    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            float dist = std::sqrt((x - cx) * (x - cx) + (y - cy) * (y - cy));
            float outer = radius + feather * radius;
            float alpha = 1.0f - std::clamp((dist - radius) / (outer - radius + 0.001f), 0.0f, 1.0f);
            if (invert) alpha = 1.0f - alpha;
            uint8_t* p = buffer.pixelAt(x, y);
            p[0] = static_cast<uint8_t>(col.r * alpha + p[0] * (1.0f - alpha));
            p[1] = static_cast<uint8_t>(col.g * alpha + p[1] * (1.0f - alpha));
            p[2] = static_cast<uint8_t>(col.b * alpha + p[2] * (1.0f - alpha));
            p[3] = static_cast<uint8_t>(std::min(static_cast<double>(alpha * 255.0f + p[3] * (1.0f - alpha)), 255.0));
        }
    }
}

} // namespace FreeEffect
