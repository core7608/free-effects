#include "../effect_registry.h"
#include "magnify_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<MagnifyEffect> s_reg("Magnify", "Distort");

MagnifyEffect::MagnifyEffect() {
    addParameter(EffectParameter::makeVec2("center", "Center", {0.5, 0.5}));
    addParameter(EffectParameter::makeFloat("magnification", "Magnification", 0.0, 5.0, 2.0));
    addParameter(EffectParameter::makeFloat("radius", "Radius", 0.0, 100.0, 30.0));
    addParameter(EffectParameter::makeFloat("feather", "Feather", 0.0, 100.0, 50.0));
}

std::vector<ParameterGroup> MagnifyEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeVec2("center", "Center", {0.5, 0.5}),
        EffectParameter::makeFloat("magnification", "Magnification", 0.0, 5.0, false),
        EffectParameter::makeFloat("radius", "Radius", 0.0, 100.0, false),
        EffectParameter::makeFloat("feather", "Feather", 0.0, 100.0, false)
    }}};
}

std::unique_ptr<Effect> MagnifyEffect::clone() const {
    auto e = std::make_unique<MagnifyEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void MagnifyEffect::render(PixelBuffer& buffer, double time) {
    Vec2 center = getVec2Param("center");
    float mag = getFloatParam("magnification");
    float rad = getFloatParam("radius") / 100.0f;
    float feather = getFloatParam("feather") / 100.0f;
    float cx = center.x * buffer.width;
    float cy = center.y * buffer.height;
    float maxR = rad * std::max(buffer.width, buffer.height);
    float innerR = maxR * (1.0f - feather);

    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            float dx = x - cx;
            float dy = y - cy;
            float dist = std::sqrt(dx * dx + dy * dy);

            if (dist < maxR) {
                float blend = (dist > innerR && feather > 0.0f)
                    ? (maxR - dist) / (maxR - innerR) : 1.0f;
                float scale = 1.0f + (mag - 1.0f) * blend;
                int sx = static_cast<int>(cx + dx / scale);
                int sy = static_cast<int>(cy + dy / scale);
                sx = std::clamp(sx, 0, buffer.width - 1);
                sy = std::clamp(sy, 0, buffer.height - 1);
                const uint8_t* src = buffer.pixelAt(sx, sy);
                uint8_t* dst = tmp.pixelAt(x, y);
                dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3];
            } else {
                const uint8_t* src = buffer.pixelAt(x, y);
                uint8_t* dst = tmp.pixelAt(x, y);
                dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3];
            }
        }
    }
    buffer.data = tmp.data;
}

} // namespace FreeEffect
