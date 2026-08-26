#include "../effect_registry.h"
#include "vector_paint_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<VectorPaintEffect> s_reg("Vector Paint", "Paint");

VectorPaintEffect::VectorPaintEffect() {
    addParameter(EffectParameter::makeFloat("brushSize", "Brush Size", 1.0, 200.0, 10.0));
    addParameter(EffectParameter::makeColor("brushColor", "Color", {255.0, 255.0, 255.0, 1.0}));
    addParameter(EffectParameter::makeFloat("opacity", "Opacity", 0.0, 100.0, 100.0));
    addParameter(EffectParameter::makeFloat("smoothing", "Smoothing", 0.0, 100.0, 50.0));
}

std::unique_ptr<Effect> VectorPaintEffect::clone() const {
    auto e = std::make_unique<VectorPaintEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void VectorPaintEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    float brushSize = getFloatParam("brushSize");
    Color col = getColorParam("brushColor");
    float opacity = getFloatParam("opacity") / 100.0f;
    float smooth = getFloatParam("smoothing") / 100.0f;

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            float hash = std::fmod(std::sin(static_cast<float>(x)*12.9898f + y*78.233f) * 43758.5453f, 1.0f);
            if (hash < smooth * 0.05f) {
                float dist = std::fmod(hash * brushSize, brushSize) / brushSize;
                float alpha = (1.0f - dist) * opacity;
                uint8_t* p = buffer.pixelAt(x, y);
                p[0] = static_cast<uint8_t>(std::clamp(static_cast<double>(p[0] * (1.0f-alpha) + col.r*alpha), 0.0, 255.0));
                p[1] = static_cast<uint8_t>(std::clamp(static_cast<double>(p[1] * (1.0f-alpha) + col.g*alpha), 0.0, 255.0));
                p[2] = static_cast<uint8_t>(std::clamp(static_cast<double>(p[2] * (1.0f-alpha) + col.b*alpha), 0.0, 255.0));
            }
        }
    }
}

} // namespace FreeEffect
