#include "../effect_registry.h"
#include "cc_light_wipe_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<CCLightWipeEffect> s_reg("CC Light Wipe", "Transition");

CCLightWipeEffect::CCLightWipeEffect() {
    addParameter(EffectParameter::makeFloat("completion", "Completion", 0.0, 100.0, 0.0));
    addParameter(EffectParameter::makeFloat("intensity", "Intensity", 0.0, 100.0, 50.0));
    addParameter(EffectParameter::makeFloat("center", "Center", 0.0, 100.0, 50.0));
}

std::unique_ptr<Effect> CCLightWipeEffect::clone() const {
    auto e = std::make_unique<CCLightWipeEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void CCLightWipeEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    float comp = getFloatParam("completion") / 100.0f;
    float intensity = getFloatParam("intensity") / 100.0f;
    float center = getFloatParam("center") / 100.0f;

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            float nx = static_cast<float>(x) / buffer.width;
            float dist = std::abs(nx - center);
            float alpha = std::clamp((comp - dist + 0.1f) / 0.2f, 0.0f, 1.0f);
            uint8_t* p = buffer.pixelAt(x, y);
            float glow = std::max(0.0f, 1.0f - std::abs(nx - (center + comp)) * 5.0f) * intensity;
            p[0] = static_cast<uint8_t>(std::min(255.0, static_cast<double>(p[0] + 255.0f * glow)));
            p[1] = static_cast<uint8_t>(std::min(255.0, static_cast<double>(p[1] + 255.0f * glow)));
            p[2] = static_cast<uint8_t>(std::min(255.0, static_cast<double>(p[2] + 255.0f * glow)));
            p[3] = static_cast<uint8_t>(alpha * 255.0f);
        }
    }
}

} // namespace FreeEffect
