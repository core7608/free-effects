#include "../effect_registry.h"
#include "transform_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<TransformEffect> s_reg("Transform", "Utility");

TransformEffect::TransformEffect() {
    addParameter(EffectParameter::makeVec2("position", "Position", {0.5, 0.5}));
    addParameter(EffectParameter::makeVec2("scale", "Scale", {1.0, 1.0}));
    addParameter(EffectParameter::makeAngle("rotation", "Rotation", 0.0));
    addParameter(EffectParameter::makeFloat("opacity", "Opacity", 0.0, 100.0, 100.0));
}

std::vector<ParameterGroup> TransformEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeVec2("position", "Position", {0.5, 0.5}),
        EffectParameter::makeVec2("scale", "Scale", {1.0, 1.0}),
        EffectParameter::makeAngle("rotation", "Rotation", 0.0),
        EffectParameter::makeFloat("opacity", "Opacity", 0.0, 100.0, false)
    }}};
}

std::unique_ptr<Effect> TransformEffect::clone() const {
    auto e = std::make_unique<TransformEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void TransformEffect::render(PixelBuffer& buffer, double time) {
    Vec2 pos = getVec2Param("position");
    Vec2 scale = getVec2Param("scale");
    float rotation = getAngleParam("rotation") * 3.14159265f / 180.0f;
    float opacity = getFloatParam("opacity") / 100.0f;

    float cx = pos.x * buffer.width;
    float cy = pos.y * buffer.height;
    float cosR = std::cos(-rotation);
    float sinR = std::sin(-rotation);
    float invSx = (scale.x != 0) ? 1.0f / scale.x : 1.0f;
    float invSy = (scale.y != 0) ? 1.0f / scale.y : 1.0f;

    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            float dx = (x - cx) * invSx;
            float dy = (y - cy) * invSy;
            float rx = dx * cosR - dy * sinR;
            float ry = dx * sinR + dy * cosR;
            int sx = static_cast<int>(cx + rx);
            int sy = static_cast<int>(cy + ry);
            uint8_t* dst = tmp.pixelAt(x, y);
            if (sx >= 0 && sx < buffer.width && sy >= 0 && sy < buffer.height) {
                const uint8_t* src = buffer.pixelAt(sx, sy);
                for (int c = 0; c < 3; c++) {
                    dst[c] = src[c];
                }
                dst[3] = static_cast<uint8_t>(src[3] * opacity);
            }
        }
    }
    buffer.data = tmp.data;
}

} // namespace FreeEffect
