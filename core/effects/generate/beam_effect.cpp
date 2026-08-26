#include "../effect_registry.h"
#include "beam_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<BeamEffect> s_reg("Beam", "Generate");

BeamEffect::BeamEffect() {
    addParameter(EffectParameter::makeVec2("startPoint", "Start", {0.0, 0.5}));
    addParameter(EffectParameter::makeVec2("endPoint", "End", {1.0, 0.5}));
    addParameter(EffectParameter::makeFloat("length", "Length", 0.0, 100.0, 50.0));
    addParameter(EffectParameter::makeFloat("thickness", "Thickness", 1.0, 50.0, 5.0));
    addParameter(EffectParameter::makeColor("insideColor", "Inside Color", {255.0, 255.0, 255.0, 1.0}));
    addParameter(EffectParameter::makeColor("outsideColor", "Outside Color", {0.0, 0.0, 255.0, 1.0}));
    addParameter(EffectParameter::makeFloat("softness", "Softness", 0.0, 100.0, 50.0));
    addParameter(EffectParameter::makeBool("insideOnly", "Inside Only", false));
}

std::unique_ptr<Effect> BeamEffect::clone() const {
    auto e = std::make_unique<BeamEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void BeamEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    Vec2 start = getVec2Param("startPoint"), end = getVec2Param("endPoint");
    float thick = getFloatParam("thickness");
    float soft = getFloatParam("softness") / 100.0f;
    Color inside = getColorParam("insideColor"), outside = getColorParam("outsideColor");

    float sx = start.x * buffer.width, sy = start.y * buffer.height;
    float ex = end.x * buffer.width, ey = end.y * buffer.height;
    float dx = ex - sx, dy = ey - sy;
    float len = std::sqrt(dx * dx + dy * dy);
    if (len < 1.0f) return;
    float nx = -dy / len, ny = dx / len;

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            float px = x - sx, py = y - sy;
            float t = (px * dx + py * dy) / (len * len);
            float dist = std::abs(px * nx + py * ny);
            t = std::clamp(t, 0.0f, 1.0f);
            float beamT = std::abs(t - 0.5f) * 2.0f;
            float inner = thick * 0.5f;
            float outer = inner * (1.0f + soft);
            if (dist < inner) {
                uint8_t* p = buffer.pixelAt(x, y);
                p[0] = static_cast<uint8_t>(inside.r);
                p[1] = static_cast<uint8_t>(inside.g);
                p[2] = static_cast<uint8_t>(inside.b);
                p[3] = static_cast<uint8_t>(255.0f * beamT);
            } else if (dist < outer) {
                float fade = 1.0f - (dist - inner) / (outer - inner);
                uint8_t* p = buffer.pixelAt(x, y);
                p[0] = static_cast<uint8_t>(outside.r * (1.0f - fade) + inside.r * fade);
                p[1] = static_cast<uint8_t>(outside.g * (1.0f - fade) + inside.g * fade);
                p[2] = static_cast<uint8_t>(outside.b * (1.0f - fade) + inside.b * fade);
                p[3] = static_cast<uint8_t>(255.0f * beamT * fade);
            }
        }
    }
}

} // namespace FreeEffect
