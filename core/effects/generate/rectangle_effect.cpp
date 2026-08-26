#include "../effect_registry.h"
#include "rectangle_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<RectangleEffect> s_reg("Rectangle", "Generate");

RectangleEffect::RectangleEffect() {
    addParameter(EffectParameter::makeVec2("center", "Center", {0.5, 0.5}));
    addParameter(EffectParameter::makeFloat("width", "Width", 0.0, 4096.0, 200.0));
    addParameter(EffectParameter::makeFloat("height", "Height", 0.0, 4096.0, 200.0));
    addParameter(EffectParameter::makeFloat("feather", "Feather", 0.0, 200.0, 0.0));
    addParameter(EffectParameter::makeFloat("thickness", "Stroke Width", 0.0, 100.0, 2.0));
}

std::vector<ParameterGroup> RectangleEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeVec2("center", "Center", {0.5, 0.5}),
        EffectParameter::makeFloat("width", "Width", 0.0, 4096.0, false),
        EffectParameter::makeFloat("height", "Height", 0.0, 4096.0, false),
        EffectParameter::makeFloat("feather", "Feather", 0.0, 200.0, false),
        EffectParameter::makeFloat("thickness", "Stroke Width", 0.0, 100.0, false)
    }}};
}

std::unique_ptr<Effect> RectangleEffect::clone() const {
    auto e = std::make_unique<RectangleEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void RectangleEffect::render(PixelBuffer& buffer, double time) {
    Vec2 center = getVec2Param("center");
    float w = getFloatParam("width") / 2.0f;
    float h = getFloatParam("height") / 2.0f;
    float feather = getFloatParam("feather");
    float thickness = getFloatParam("thickness");
    float cx = center.x * buffer.width;
    float cy = center.y * buffer.height;

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            float dx = std::abs(x - cx);
            float dy = std::abs(y - cy);
            float alpha = 0.0f;
            if (thickness > 0 && w > 0 && h > 0) {
                if (dx <= w && dy <= h) {
                    float edgeDist = std::min(std::min(w - dx, h - dy), std::min(dx, dy));
                    if (dx >= w - thickness || dy >= h - thickness) {
                        alpha = 1.0f;
                        if (feather > 0) {
                            float minEdge = std::min(w - dx, h - dy);
                            alpha = std::clamp(minEdge / feather, 0.0f, 1.0f);
                        }
                    }
                }
            } else {
                if (dx <= w && dy <= h) {
                    alpha = 1.0f;
                    if (feather > 0) {
                        float minEdge = std::min(w - dx, h - dy);
                        alpha = std::clamp(minEdge / feather, 0.0f, 1.0f);
                    }
                }
            }
            if (alpha > 0.0f) {
                uint8_t* p = buffer.pixelAt(x, y);
                p[3] = static_cast<uint8_t>(std::clamp(static_cast<double>(alpha * 255.0f), 0.0, 255.0));
            }
        }
    }
}

} // namespace FreeEffect
