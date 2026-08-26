#include "../effect_registry.h"
#include "simple_choker_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<SimpleChokerEffect> s_reg("Simple Choker", "Keying");

SimpleChokerEffect::SimpleChokerEffect() {
    addParameter(EffectParameter::makeFloat("chokeMatte", "Choke Matte", -100.0, 100.0, 0.0));
}

std::unique_ptr<Effect> SimpleChokerEffect::clone() const {
    auto e = std::make_unique<SimpleChokerEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void SimpleChokerEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    float choke = getFloatParam("chokeMatte") / 100.0f;
    int radius = static_cast<int>(std::abs(choke) * 5);

    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);
    std::copy(buffer.data.begin(), buffer.data.end(), tmp.data.begin());

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            const uint8_t* p = tmp.pixelAt(x, y);
            float a = p[3] / 255.0f;
            if (radius > 0) {
                float best = a;
                for (int dy = -radius; dy <= radius; dy++) {
                    for (int dx = -radius; dx <= radius; dx++) {
                        int sx = std::clamp(x + dx, 0, buffer.width - 1);
                        int sy = std::clamp(y + dy, 0, buffer.height - 1);
                        float na = tmp.pixelAt(sx, sy)[3] / 255.0f;
                        if (choke > 0) best = std::min(best, na);
                        else best = std::max(best, na);
                    }
                }
                a = best;
            }
            uint8_t* dst = buffer.pixelAt(x, y);
            dst[3] = static_cast<uint8_t>(std::clamp(a * 255.0f, 0.0f, 255.0f));
        }
    }
}

} // namespace FreeEffect
