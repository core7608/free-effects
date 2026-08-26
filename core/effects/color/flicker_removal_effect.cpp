#include "../effect_registry.h"
#include "flicker_removal_effect.h"
#include <algorithm>
#include <cmath>
#include <vector>

namespace FreeEffect {

static EffectRegistrar<FlickerRemovalEffect> s_reg("Flicker Removal", "Color Correction");

FlickerRemovalEffect::FlickerRemovalEffect() {
    addParameter(EffectParameter::makeInt("frameRange", "Smoothing Frame Range", 1, 30, 5));
    addParameter(EffectParameter::makeFloat("strength", "Strength", 0.0, 100.0, 100.0));
}

std::unique_ptr<Effect> FlickerRemovalEffect::clone() const {
    auto e = std::make_unique<FlickerRemovalEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void FlickerRemovalEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    int frameRange = getIntParam("frameRange");
    float strength = getFloatParam("strength") / 100.0f;

    float totalLuma = 0;
    int count = 0;
    for (int y = 0; y < buffer.height; y += 4) {
        for (int x = 0; x < buffer.width; x += 4) {
            const uint8_t* p = buffer.pixelAt(x, y);
            totalLuma += 0.299f * p[0] + 0.587f * p[1] + 0.114f * p[2];
            count++;
        }
    }
    float avgLuma = (count > 0) ? totalLuma / count : 128.0f;
    float targetLuma = 128.0f;
    float gain = (avgLuma > 1.0f) ? targetLuma / avgLuma : 1.0f;
    gain = 1.0f + (gain - 1.0f) * strength;

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            p[0] = static_cast<uint8_t>(std::clamp(static_cast<double>(p[0] * gain), 0.0, 255.0));
            p[1] = static_cast<uint8_t>(std::clamp(static_cast<double>(p[1] * gain), 0.0, 255.0));
            p[2] = static_cast<uint8_t>(std::clamp(static_cast<double>(p[2] * gain), 0.0, 255.0));
        }
    }
}

} // namespace FreeEffect
