#include "../effect_registry.h"
#include "pro_amp_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<ProAmpEffect> s_reg("Pro Amp", "Utility");

ProAmpEffect::ProAmpEffect() {
    addParameter(EffectParameter::makeFloat("gain", "Gain", 0.0, 200.0, 100.0));
    addParameter(EffectParameter::makeFloat("pedestal", "Pedestal", -100.0, 100.0, 0.0));
    addParameter(EffectParameter::makeBool("broadcastSafe", "Broadcast Safe", false));
}

std::unique_ptr<Effect> ProAmpEffect::clone() const {
    auto e = std::make_unique<ProAmpEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void ProAmpEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    float gain = getFloatParam("gain") / 100.0f;
    float ped = getFloatParam("pedestal") / 100.0f * 255.0f;
    bool safe = getBoolParam("broadcastSafe");

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            for (int c = 0; c < 3; c++) {
                float v = p[c] * gain + ped;
                if (safe) v = std::clamp(v, 16.0f, 235.0f);
                else v = std::clamp(v, 0.0f, 255.0f);
                p[c] = static_cast<uint8_t>(v);
            }
        }
    }
}

} // namespace FreeEffect
