#include "../effect_registry.h"
#include "cc_scale_wipe_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<CCScaleWipeEffect> s_reg("CC Scale Wipe", "Transition");

CCScaleWipeEffect::CCScaleWipeEffect() {
    addParameter(EffectParameter::makeFloat("completion", "Completion", 0.0, 100.0, 0.0));
    addParameter(EffectParameter::makeVec2("center", "Center", {0.5, 0.5}));
    addParameter(EffectParameter::makeAngle("direction", "Direction", 0.0));
}

std::unique_ptr<Effect> CCScaleWipeEffect::clone() const {
    auto e = std::make_unique<CCScaleWipeEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void CCScaleWipeEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    float comp = getFloatParam("completion") / 100.0f;
    Vec2 center = getVec2Param("center");
    float dir = getFloatParam("direction") * 3.14159265f / 180.0f;

    float cx = center.x, cy = center.y;
    float cosD = std::cos(dir), sinD = std::sin(dir);

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            float nx = static_cast<float>(x) / buffer.width - cx;
            float ny = static_cast<float>(y) / buffer.height - cy;
            float proj = nx * cosD + ny * sinD;
            float scale = 1.0f + comp * 2.0f;
            float alpha = std::clamp((proj + 0.5f - comp) / 0.3f, 0.0f, 1.0f);
            uint8_t* p = buffer.pixelAt(x, y);
            p[3] = static_cast<uint8_t>(alpha * 255.0f);
        }
    }
}

} // namespace FreeEffect
