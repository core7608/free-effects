#include "../effect_registry.h"
#include "ellipse_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<EllipseEffect> s_reg("Ellipse", "Generate");

EllipseEffect::EllipseEffect() {
    addParameter(EffectParameter::makeVec2("center", "Center", {0.5, 0.5}));
    addParameter(EffectParameter::makeFloat("width", "Width", 0.0, 4096.0, 200.0));
    addParameter(EffectParameter::makeFloat("height", "Height", 0.0, 4096.0, 200.0));
    addParameter(EffectParameter::makeFloat("feather", "Feather", 0.0, 200.0, 0.0));
    addParameter(EffectParameter::makeFloat("thickness", "Thickness", 0.0, 100.0, 2.0));
}

std::vector<ParameterGroup> EllipseEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeVec2("center", "Center", {0.5, 0.5}),
        EffectParameter::makeFloat("width", "Width", 0.0, 4096.0, false),
        EffectParameter::makeFloat("height", "Height", 0.0, 4096.0, false),
        EffectParameter::makeFloat("feather", "Feather", 0.0, 200.0, false),
        EffectParameter::makeFloat("thickness", "Thickness", 0.0, 100.0, false)
    }}};
}

std::unique_ptr<Effect> EllipseEffect::clone() const {
    auto e = std::make_unique<EllipseEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void EllipseEffect::render(PixelBuffer& buffer, double time) {
    Vec2 center = getVec2Param("center");
    float w = getFloatParam("width") / 2.0f;
    float h = getFloatParam("height") / 2.0f;
    float feather = getFloatParam("feather");
    float thickness = getFloatParam("thickness");
    float cx = center.x * buffer.width;
    float cy = center.y * buffer.height;

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            float dx = (x - cx) / std::max(w, 0.001f);
            float dy = (y - cy) / std::max(h, 0.001f);
            float dist = std::sqrt(dx * dx + dy * dy);
            float alpha = 0.0f;
            if (thickness > 0 && w > 0 && h > 0) {
                float outerD = dist;
                float innerR = 1.0f - thickness / std::max(w, h);
                if (outerD <= 1.0f) {
                    if (innerR <= 0 || dist >= innerR) {
                        alpha = 1.0f;
                        if (feather > 0) {
                            float edgeDist = std::min(1.0f - dist, dist - innerR);
                            alpha = std::clamp(edgeDist / feather, 0.0f, 1.0f);
                        }
                    }
                }
            } else {
                if (dist <= 1.0f) {
                    alpha = 1.0f;
                    if (feather > 0) {
                        alpha = std::clamp((1.0f - dist) / (feather / std::max(w, h)), 0.0f, 1.0f);
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
