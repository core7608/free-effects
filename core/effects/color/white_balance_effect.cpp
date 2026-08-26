#include "../effect_registry.h"
#include "white_balance_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<WhiteBalanceEffect> s_reg("White Balance", "Color Correction");

WhiteBalanceEffect::WhiteBalanceEffect() {
    addParameter(EffectParameter::makeColor("whitePoint", "White Point", {255.0, 255.0, 255.0, 1.0}));
    addParameter(EffectParameter::makeFloat("temperature", "Temperature", -100.0, 100.0, 0.0));
}

std::unique_ptr<Effect> WhiteBalanceEffect::clone() const {
    auto e = std::make_unique<WhiteBalanceEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void WhiteBalanceEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    Color wp = getColorParam("whitePoint");
    float temperature = getFloatParam("temperature") / 100.0f;

    float scaleR = (wp.r > 1.0f) ? 255.0f / wp.r : 1.0f;
    float scaleG = (wp.g > 1.0f) ? 255.0f / wp.g : 1.0f;
    float scaleB = (wp.b > 1.0f) ? 255.0f / wp.b : 1.0f;

    float tempR = 1.0f + temperature * 0.1f;
    float tempB = 1.0f - temperature * 0.1f;

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            float r = p[0] * scaleR * tempR;
            float g = p[1] * scaleG;
            float b = p[2] * scaleB * tempB;
            p[0] = static_cast<uint8_t>(std::clamp(r, 0.0f, 255.0f));
            p[1] = static_cast<uint8_t>(std::clamp(g, 0.0f, 255.0f));
            p[2] = static_cast<uint8_t>(std::clamp(b, 0.0f, 255.0f));
        }
    }
}

} // namespace FreeEffect
