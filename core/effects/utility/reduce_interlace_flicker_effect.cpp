#include "../effect_registry.h"
#include "reduce_interlace_flicker_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<ReduceInterlaceFlickerEffect> s_reg("Reduce Interlace Flicker", "Utility");

ReduceInterlaceFlickerEffect::ReduceInterlaceFlickerEffect() {
    addParameter(EffectParameter::makeFloat("softness", "Softness", 0.0, 100.0, 50.0));
}

std::unique_ptr<Effect> ReduceInterlaceFlickerEffect::clone() const {
    auto e = std::make_unique<ReduceInterlaceFlickerEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void ReduceInterlaceFlickerEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    float softness = getFloatParam("softness") / 100.0f;
    int radius = static_cast<int>(softness * 2) + 1;

    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);
    std::copy(buffer.data.begin(), buffer.data.end(), tmp.data.begin());

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            float rS = 0, gS = 0, bS = 0; int count = 0;
            for (int dy = -radius; dy <= radius; dy++) {
                int sy = std::clamp(y + dy, 0, buffer.height - 1);
                const uint8_t* p = tmp.pixelAt(x, sy);
                rS += p[0]; gS += p[1]; bS += p[2]; count++;
            }
            uint8_t* dst = buffer.pixelAt(x, y);
            dst[0] = static_cast<uint8_t>(std::clamp(rS / count, 0.0f, 255.0f));
            dst[1] = static_cast<uint8_t>(std::clamp(gS / count, 0.0f, 255.0f));
            dst[2] = static_cast<uint8_t>(std::clamp(bS / count, 0.0f, 255.0f));
        }
    }
}

} // namespace FreeEffect
