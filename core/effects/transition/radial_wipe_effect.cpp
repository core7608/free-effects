#include "../effect_registry.h"
#include "radial_wipe_effect.h"
#include <cmath>
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<RadialWipeEffect> s_reg("Radial Wipe", "Transition");

RadialWipeEffect::RadialWipeEffect() {
    addParameter(EffectParameter::makeFloat("progress", "Progress", 0.0, 1.0, 0.0));
    addParameter(EffectParameter::makeVec2("center", "Center", Vec2{0.5, 0.5}));
    addParameter(EffectParameter::makeAngle("start_angle", "Start Angle", 0.0));
    addParameter(EffectParameter::makeFloat("feather", "Feather", 0.0, 30.0, 3.0));
}

std::vector<ParameterGroup> RadialWipeEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeFloat("progress", "Progress", 0.0, 1.0, 0.0),
        EffectParameter::makeVec2("center", "Center", Vec2{0.5, 0.5}),
        EffectParameter::makeAngle("start_angle", "Start Angle", 0.0),
        EffectParameter::makeFloat("feather", "Feather", 0.0, 30.0, 3.0)
    }}};
}

std::unique_ptr<Effect> RadialWipeEffect::clone() const {
    auto e = std::make_unique<RadialWipeEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void RadialWipeEffect::render(PixelBuffer& buffer, double time) {
    if (buffer.width == 0 || buffer.height == 0) return;
    double progress = getFloatParam("progress");
    Vec2 ctr = getVec2Param("center");
    double startAngle = getAngleParam("start_angle") * M_PI / 180.0;
    double feather = getFloatParam("feather") * M_PI / 180.0;
    double cx = ctr.x * buffer.width;
    double cy = ctr.y * buffer.height;
    double wipeAngle = progress * 6.28318;
    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            double dx = x - cx;
            double dy = y - cy;
            double angle = std::atan2(dy, dx) - startAngle;
            while (angle < 0) angle += 6.28318;
            while (angle >= 6.28318) angle -= 6.28318;
            double edgeDist = wipeAngle - angle;
            if (edgeDist < 0) edgeDist += 6.28318;
            double alpha = std::clamp(edgeDist / (feather + 0.001), 0.0, 1.0);
            if (alpha > 0.999) continue;
            uint8_t* p = buffer.pixelAt(x, y);
            p[0] = static_cast<uint8_t>(p[0] * alpha);
            p[1] = static_cast<uint8_t>(p[1] * alpha);
            p[2] = static_cast<uint8_t>(p[2] * alpha);
            p[3] = static_cast<uint8_t>(p[3] * alpha);
        }
    }
}

} // namespace FreeEffect
