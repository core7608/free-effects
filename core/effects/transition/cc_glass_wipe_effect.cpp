#include "../effect_registry.h"
#include "cc_glass_wipe_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<CCGlassWipeEffect> s_reg("CC Glass Wipe", "Transition");

CCGlassWipeEffect::CCGlassWipeEffect() {
    addParameter(EffectParameter::makeFloat("completion", "Completion", 0.0, 100.0, 0.0));
    addParameter(EffectParameter::makeFloat("softness", "Softness", 0.0, 100.0, 50.0));
    addParameter(EffectParameter::makeFloat("turbulence", "Turbulence", 0.0, 100.0, 50.0));
}

std::unique_ptr<Effect> CCGlassWipeEffect::clone() const {
    auto e = std::make_unique<CCGlassWipeEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void CCGlassWipeEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    float comp = getFloatParam("completion") / 100.0f;
    float soft = getFloatParam("softness") / 100.0f;
    float turb = getFloatParam("turbulence") / 100.0f;

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            float nx = static_cast<float>(x) / buffer.width;
            float turbDisp = std::sin(nx * 20.0f + y * 0.1f) * turb * 0.1f;
            float edge = comp + turbDisp;
            float alpha = std::clamp((edge - nx + soft) / (soft * 2.0f + 0.001f), 0.0f, 1.0f);
            uint8_t* p = buffer.pixelAt(x, y);
            p[3] = static_cast<uint8_t>(alpha * 255.0f);
        }
    }
}

} // namespace FreeEffect
