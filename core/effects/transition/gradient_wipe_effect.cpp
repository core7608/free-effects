#include "../../math/math_constants.h"
#include "../effect_registry.h"
#include "gradient_wipe_effect.h"
#include <cmath>
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<GradientWipeEffect> s_reg("Gradient Wipe", "Transition");

GradientWipeEffect::GradientWipeEffect() {
    addParameter(EffectParameter::makeFloat("progress", "Progress", 0.0, 1.0, 0.0));
    addParameter(EffectParameter::makeAngle("direction", "Direction", 0.0));
    addParameter(EffectParameter::makeFloat("softness", "Softness", 0.0, 50.0, 10.0));
    addParameter(EffectParameter::makeBool("invert", "Invert", false));
}

std::vector<ParameterGroup> GradientWipeEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeFloat("progress", "Progress", 0.0, 1.0, 0.0),
        EffectParameter::makeAngle("direction", "Direction", 0.0),
        EffectParameter::makeFloat("softness", "Softness", 0.0, 50.0, 10.0),
        EffectParameter::makeBool("invert", "Invert", false)
    }}};
}

std::unique_ptr<Effect> GradientWipeEffect::clone() const {
    auto e = std::make_unique<GradientWipeEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void GradientWipeEffect::render(PixelBuffer& buffer, double time) {
    if (buffer.width == 0 || buffer.height == 0) return;
    double progress = getFloatParam("progress");
    double dir = getAngleParam("direction") * M_PI / 180.0;
    double softness = getFloatParam("softness");
    bool inv = getBoolParam("invert");
    double nx = std::cos(dir);
    double ny = std::sin(dir);
    double cx = buffer.width / 2.0;
    double cy = buffer.height / 2.0;
    double maxProj = std::abs(nx * cx) + std::abs(ny * cy);
    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            double dx = x - cx;
            double dy = y - cy;
            double proj = (dx * nx + dy * ny) / maxProj;
            double gradVal = (proj + 1.0) * 0.5;
            if (inv) gradVal = 1.0 - gradVal;
            double edgeDist = gradVal - progress;
            double alpha = std::clamp((edgeDist + softness / maxProj) / (softness * 2.0 / maxProj + 0.001), 0.0, 1.0);
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
