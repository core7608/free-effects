#include "../effect_registry.h"
#include "hoop_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<HoopEffect> s_reg("Hoop", "Generate");

HoopEffect::HoopEffect() {
    addParameter(EffectParameter::makeVec2("center", "Center", {0.5, 0.5}));
    addParameter(EffectParameter::makeFloat("radius", "Radius", 0.0, 1000.0, 100.0));
    addParameter(EffectParameter::makeFloat("width", "Width", 1.0, 100.0, 5.0));
    addParameter(EffectParameter::makeColor("color", "Color", {255.0, 255.0, 255.0, 1.0}));
    addParameter(EffectParameter::makeAngle("rotation", "Rotation", 0.0));
    addParameter(EffectParameter::makeFloat("feather", "Feather", 0.0, 100.0, 0.0));
}

std::unique_ptr<Effect> HoopEffect::clone() const {
    auto e = std::make_unique<HoopEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void HoopEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    Vec2 center = getVec2Param("center");
    float radius = getFloatParam("radius");
    float w = getFloatParam("width");
    Color col = getColorParam("color");
    float feather = getFloatParam("feather") / 100.0f;

    float cx = center.x * buffer.width, cy = center.y * buffer.height;
    float inner = radius - w * 0.5f;
    float outer = radius + w * 0.5f;
    float featherW = feather * w;

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            float dist = std::sqrt((x-cx)*(x-cx) + (y-cy)*(y-cy));
            float alpha = 0;
            if (dist >= inner - featherW && dist <= outer + featherW) {
                if (dist < inner) alpha = (dist - (inner - featherW)) / (featherW + 0.001f);
                else if (dist > outer) alpha = 1.0f - (dist - outer) / (featherW + 0.001f);
                else alpha = 1.0f;
                alpha = std::clamp(alpha, 0.0f, 1.0f);
            }
            if (alpha > 0) {
                uint8_t* p = buffer.pixelAt(x, y);
                p[0] = static_cast<uint8_t>(std::clamp(static_cast<double>(p[0]*(1-alpha) + col.r*alpha), 0.0, 255.0));
                p[1] = static_cast<uint8_t>(std::clamp(static_cast<double>(p[1]*(1-alpha) + col.g*alpha), 0.0, 255.0));
                p[2] = static_cast<uint8_t>(std::clamp(static_cast<double>(p[2]*(1-alpha) + col.b*alpha), 0.0, 255.0));
                p[3] = static_cast<uint8_t>(std::min(255.0, static_cast<double>(p[3] + alpha * 255.0f)));
            }
        }
    }
}

} // namespace FreeEffect
