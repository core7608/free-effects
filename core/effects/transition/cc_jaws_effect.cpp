#include "../effect_registry.h"
#include "cc_jaws_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<CCJawsEffect> s_reg("CC Jaws", "Transition");

CCJawsEffect::CCJawsEffect() {
    addParameter(EffectParameter::makeFloat("completion", "Completion", 0.0, 100.0, 0.0));
    addParameter(EffectParameter::makeInt("teeth", "Teeth", 2, 20, 6));
    addParameter(EffectParameter::makeFloat("direction", "Direction", 0.0, 360.0, 90.0));
}

std::unique_ptr<Effect> CCJawsEffect::clone() const {
    auto e = std::make_unique<CCJawsEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void CCJawsEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    float comp = getFloatParam("completion") / 100.0f;
    int teeth = getIntParam("teeth");
    float dir = getFloatParam("direction") * 3.14159265f / 180.0f;

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            float nx = static_cast<float>(x) / buffer.width;
            float ny = static_cast<float>(y) / buffer.height;
            float wave = std::sin(ny * teeth * 3.14159265f) * 0.15f;
            float edge = comp + wave;
            float alpha = std::clamp((edge - nx + 0.1f) / 0.2f, 0.0f, 1.0f);
            uint8_t* p = buffer.pixelAt(x, y);
            p[3] = static_cast<uint8_t>(alpha * 255.0f);
        }
    }
}

} // namespace FreeEffect
