#include "../effect_registry.h"
#include "bulge_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<BulgeEffect> s_reg("Bulge", "Distort");

BulgeEffect::BulgeEffect() {
    addParameter(EffectParameter::makeVec2("center", "Center", {0.5, 0.5}));
    addParameter(EffectParameter::makeFloat("amount", "Amount", -1.0, 1.0, 0.5));
    addParameter(EffectParameter::makeFloat("radius", "Radius", 0.0, 100.0, 50.0));
}

std::vector<ParameterGroup> BulgeEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeVec2("center", "Center", {0.5, 0.5}),
        EffectParameter::makeFloat("amount", "Amount", -1.0, 1.0, false),
        EffectParameter::makeFloat("radius", "Radius", 0.0, 100.0, false)
    }}};
}

std::unique_ptr<Effect> BulgeEffect::clone() const {
    auto e = std::make_unique<BulgeEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void BulgeEffect::render(PixelBuffer& buffer, double time) {
    Vec2 center = getVec2Param("center");
    float amount = getFloatParam("amount");
    float rad = getFloatParam("radius") / 100.0f;

    float cx = center.x * buffer.width;
    float cy = center.y * buffer.height;
    float maxR = rad * std::max(buffer.width, buffer.height);

    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            float dx = x - cx;
            float dy = y - cy;
            float dist = std::sqrt(dx * dx + dy * dy);
            float factor = 1.0f;
            if (dist < maxR && maxR > 0) {
                float norm = dist / maxR;
                factor = 1.0f - std::pow(1.0f - norm, 2.0f) * amount;
            }
            int sx = static_cast<int>(cx + dx * factor);
            int sy = static_cast<int>(cy + dy * factor);
            sx = std::clamp(sx, 0, buffer.width - 1);
            sy = std::clamp(sy, 0, buffer.height - 1);
            const uint8_t* src = buffer.pixelAt(sx, sy);
            uint8_t* dst = tmp.pixelAt(x, y);
            dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3];
        }
    }
    buffer.data = tmp.data;
}

} // namespace FreeEffect
