#include "../effect_registry.h"
#include "directional_wipe_effect.h"
#include <cmath>
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<DirectionalWipeEffect> s_reg("Directional Wipe", "Transition");

DirectionalWipeEffect::DirectionalWipeEffect() {
    addParameter(EffectParameter::makeFloat("progress", "Progress", 0.0, 1.0, 0.0));
    addParameter(EffectParameter::makeAngle("direction", "Direction", 0.0));
    addParameter(EffectParameter::makeFloat("feather", "Feather", 0.0, 100.0, 10.0));
}

std::vector<ParameterGroup> DirectionalWipeEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeFloat("progress", "Progress", 0.0, 1.0, 0.0),
        EffectParameter::makeAngle("direction", "Direction", 0.0),
        EffectParameter::makeFloat("feather", "Feather", 0.0, 100.0, 10.0)
    }}};
}

std::unique_ptr<Effect> DirectionalWipeEffect::clone() const {
    auto e = std::make_unique<DirectionalWipeEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void DirectionalWipeEffect::render(PixelBuffer& buffer, double time) {
    if (buffer.width == 0 || buffer.height == 0) return;
    double progress = getFloatParam("progress");
    double dir = getAngleParam("direction") * M_PI / 180.0;
    double feather = getFloatParam("feather");
    double nx = std::cos(dir);
    double ny = std::sin(dir);
    double cx = buffer.width / 2.0;
    double cy = buffer.height / 2.0;
    double maxProj = std::abs(nx * cx) + std::abs(ny * cy);
    double wipePos = progress * (maxProj * 2.0 + feather * 2.0) - maxProj - feather;
    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            double dx = x - cx;
            double dy = y - cy;
            double proj = dx * nx + dy * ny;
            double edgeDist = proj - wipePos;
            double alpha = std::clamp((edgeDist + feather) / (feather * 2.0 + 1.0), 0.0, 1.0);
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
