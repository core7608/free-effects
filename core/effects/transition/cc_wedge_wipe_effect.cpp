#include "../effect_registry.h"
#include "cc_wedge_wipe_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<CCWedgeWipeEffect> s_reg("CC Wedge Wipe", "Transition");

CCWedgeWipeEffect::CCWedgeWipeEffect() {
    addParameter(EffectParameter::makeFloat("completion", "Completion", 0.0, 100.0, 0.0));
    addParameter(EffectParameter::makeVec2("center", "Center", {0.5, 0.5}));
    addParameter(EffectParameter::makeAngle("direction", "Direction", 90.0));
    addParameter(EffectParameter::makeFloat("softness", "Softness", 0.0, 100.0, 10.0));
}

std::unique_ptr<Effect> CCWedgeWipeEffect::clone() const {
    auto e = std::make_unique<CCWedgeWipeEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void CCWedgeWipeEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    float comp = getFloatParam("completion") / 100.0f;
    Vec2 center = getVec2Param("center");
    float dir = getFloatParam("direction") * 3.14159265f / 180.0f;
    float soft = getFloatParam("softness") / 100.0f;

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            float nx = static_cast<float>(x) / buffer.width - center.x;
            float ny = static_cast<float>(y) / buffer.height - center.y;
            float angle = std::atan2(ny, nx) - dir;
            if (angle < 0) angle += 2.0f * 3.14159265f;
            float normAngle = angle / (2.0f * 3.14159265f);
            float alpha = std::clamp((comp - normAngle + soft) / (soft * 2.0f + 0.001f), 0.0f, 1.0f);
            uint8_t* p = buffer.pixelAt(x, y);
            p[3] = static_cast<uint8_t>(alpha * 255.0f);
        }
    }
}

} // namespace FreeEffect
