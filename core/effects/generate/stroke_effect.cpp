#include "../effect_registry.h"
#include "stroke_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<StrokeEffect> s_reg("Stroke", "Generate");

StrokeEffect::StrokeEffect() {
    addParameter(EffectParameter::makeColor("color", "Color", {255.0, 0.0, 0.0, 1.0}));
    addParameter(EffectParameter::makeFloat("brushSize", "Brush Size", 1.0, 200.0, 5.0));
    addParameter(EffectParameter::makeFloat("brushHardness", "Brush Hardness", 0.0, 100.0, 100.0));
    addParameter(EffectParameter::makeDropdown("pathType", "Path Type", {"All Masks", "Current"}, 0));
}

std::vector<ParameterGroup> StrokeEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeColor("color", "Color", {255.0, 0.0, 0.0, 1.0}),
        EffectParameter::makeFloat("brushSize", "Brush Size", 1.0, 200.0, false),
        EffectParameter::makeFloat("brushHardness", "Brush Hardness", 0.0, 100.0, false),
        EffectParameter::makeDropdown("pathType", "Path Type", {"All Masks", "Current"}, 0)
    }}};
}

std::unique_ptr<Effect> StrokeEffect::clone() const {
    auto e = std::make_unique<StrokeEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void StrokeEffect::render(PixelBuffer& buffer, double time) {
    Color c = getColorParam("color");
    float size = getFloatParam("brushSize");
    float hardness = getFloatParam("brushHardness") / 100.0f;
    int radius = static_cast<int>(size / 2.0f);
    float cx = buffer.width / 2.0f;
    float cy = buffer.height / 2.0f;

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            float dx = x - cx;
            float dy = y - cy;
            float dist = std::sqrt(dx * dx + dy * dy);
            float innerR = radius * hardness;
            float alpha = 0.0f;
            if (dist <= radius) {
                alpha = (dist <= innerR) ? 1.0f : 1.0f - (dist - innerR) / std::max(radius - innerR, 1.0f);
            }
            if (alpha > 0.0f) {
                uint8_t* p = buffer.pixelAt(x, y);
                float srcA = p[3] / 255.0f;
                float outA = alpha + srcA * (1.0f - alpha);
                if (outA > 0) {
                    p[0] = static_cast<uint8_t>((c.r * alpha + p[0] * srcA * (1.0f - alpha)) / outA);
                    p[1] = static_cast<uint8_t>((c.g * alpha + p[1] * srcA * (1.0f - alpha)) / outA);
                    p[2] = static_cast<uint8_t>((c.b * alpha + p[2] * srcA * (1.0f - alpha)) / outA);
                }
                p[3] = static_cast<uint8_t>(outA * 255.0f);
            }
        }
    }
}

} // namespace FreeEffect
