#include "../effect_registry.h"
#include "wipe_left_effect.h"
#include <cmath>
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<WipeLeftEffect> s_reg("Wipe Left", "Transition");

WipeLeftEffect::WipeLeftEffect() {
    addParameter(EffectParameter::makeFloat("progress", "Progress", 0.0, 1.0, 0.0));
    addParameter(EffectParameter::makeFloat("feather", "Feather", 0.0, 100.0, 10.0));
}

std::vector<ParameterGroup> WipeLeftEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeFloat("progress", "Progress", 0.0, 1.0, 0.0),
        EffectParameter::makeFloat("feather", "Feather", 0.0, 100.0, 10.0)
    }}};
}

std::unique_ptr<Effect> WipeLeftEffect::clone() const {
    auto e = std::make_unique<WipeLeftEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void WipeLeftEffect::render(PixelBuffer& buffer, double time) {
    if (buffer.width == 0 || buffer.height == 0) return;
    double progress = getFloatParam("progress");
    double feather = getFloatParam("feather");
    double wipeX = progress * (buffer.width + feather * 2) - feather;
    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            double dist = wipeX - x;
            double alpha = std::clamp((dist + feather) / (feather * 2.0 + 1.0), 0.0, 1.0);
            if (alpha > 0.999) continue;
            uint8_t* p = buffer.pixelAt(x, y);
            double fade = 1.0 - alpha;
            p[0] = static_cast<uint8_t>(p[0] * alpha);
            p[1] = static_cast<uint8_t>(p[1] * alpha);
            p[2] = static_cast<uint8_t>(p[2] * alpha);
            p[3] = static_cast<uint8_t>(p[3] * alpha);
        }
    }
}

} // namespace FreeEffect
