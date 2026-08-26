#include "../../math/math_constants.h"
#include "../effect_registry.h"
#include "venetian_blinds_effect.h"
#include <cmath>
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<VenetianBlindsEffect> s_reg("Venetian Blinds", "Transition");

VenetianBlindsEffect::VenetianBlindsEffect() {
    addParameter(EffectParameter::makeFloat("progress", "Progress", 0.0, 1.0, 0.0));
    addParameter(EffectParameter::makeInt("blinds", "Blind Count", 2, 50, 10));
    addParameter(EffectParameter::makeAngle("direction", "Direction", 0.0));
    addParameter(EffectParameter::makeFloat("feather", "Feather", 0.0, 20.0, 3.0));
}

std::vector<ParameterGroup> VenetianBlindsEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeFloat("progress", "Progress", 0.0, 1.0, 0.0),
        EffectParameter::makeInt("blinds", "Blind Count", 2, 50, 10),
        EffectParameter::makeAngle("direction", "Direction", 0.0),
        EffectParameter::makeFloat("feather", "Feather", 0.0, 20.0, 3.0)
    }}};
}

std::unique_ptr<Effect> VenetianBlindsEffect::clone() const {
    auto e = std::make_unique<VenetianBlindsEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void VenetianBlindsEffect::render(PixelBuffer& buffer, double time) {
    if (buffer.width == 0 || buffer.height == 0) return;
    double progress = getFloatParam("progress");
    int blinds = getIntParam("blinds");
    double dir = getAngleParam("direction") * M_PI / 180.0;
    double feather = getFloatParam("feather");
    double nx = std::cos(dir);
    double ny = std::sin(dir);
    double cx = buffer.width / 2.0;
    double cy = buffer.height / 2.0;
    double maxProj = std::abs(nx * cx) + std::abs(ny * cy);
    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            double dx = x - cx;
            double dy = y - cy;
            double proj = (dx * nx + dy * ny + maxProj) / (maxProj * 2.0);
            double blindPhase = std::fmod(proj * blinds, 1.0);
            double blindOpen = std::clamp((blindPhase - progress * 0.5 + feather / buffer.width) / (feather / buffer.width + 0.01), 0.0, 1.0);
            if (blindOpen > 0.999) continue;
            uint8_t* p = buffer.pixelAt(x, y);
            p[0] = static_cast<uint8_t>(p[0] * blindOpen);
            p[1] = static_cast<uint8_t>(p[1] * blindOpen);
            p[2] = static_cast<uint8_t>(p[2] * blindOpen);
            p[3] = static_cast<uint8_t>(p[3] * blindOpen);
        }
    }
}

} // namespace FreeEffect
