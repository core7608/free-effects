#include "../effect_registry.h"
#include "paint_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<PaintEffect> s_reg("Paint", "Paint");

PaintEffect::PaintEffect() {
    addParameter(EffectParameter::makeInt("brushSize", "Brush Size", 1, 200, 10));
    addParameter(EffectParameter::makeColor("brushColor", "Brush Color", {255.0, 0.0, 0.0, 1.0}));
    addParameter(EffectParameter::makeFloat("opacity", "Opacity", 0.0, 100.0, 100.0));
    addParameter(EffectParameter::makeVec2("brushPosition", "Brush Position", {0.5, 0.5}));
}

std::unique_ptr<Effect> PaintEffect::clone() const {
    auto e = std::make_unique<PaintEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void PaintEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    int brushSize = getIntParam("brushSize");
    Color col = getColorParam("brushColor");
    float opacity = getFloatParam("opacity") / 100.0f;
    Vec2 pos = getVec2Param("brushPosition");

    int cx = static_cast<int>(pos.x * buffer.width);
    int cy = static_cast<int>(pos.y * buffer.height);

    for (int dy = -brushSize; dy <= brushSize; dy++) {
        for (int dx = -brushSize; dx <= brushSize; dx++) {
            if (dx*dx + dy*dy > brushSize*brushSize) continue;
            int px = cx + dx, py = cy + dy;
            if (px < 0 || px >= buffer.width || py < 0 || py >= buffer.height) continue;
            float dist = std::sqrt(static_cast<float>(dx*dx+dy*dy)) / brushSize;
            float alpha = (1.0f - dist) * opacity;
            uint8_t* p = buffer.pixelAt(px, py);
            p[0] = static_cast<uint8_t>(std::clamp(static_cast<double>(p[0] * (1.0f-alpha) + col.r*alpha), 0.0, 255.0));
            p[1] = static_cast<uint8_t>(std::clamp(static_cast<double>(p[1] * (1.0f-alpha) + col.g*alpha), 0.0, 255.0));
            p[2] = static_cast<uint8_t>(std::clamp(static_cast<double>(p[2] * (1.0f-alpha) + col.b*alpha), 0.0, 255.0));
            p[3] = static_cast<uint8_t>(std::min(255.0, static_cast<double>(p[3] + alpha * 255.0f)));
        }
    }
}

} // namespace FreeEffect
