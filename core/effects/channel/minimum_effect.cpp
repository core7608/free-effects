#include "../effect_registry.h"
#include "minimum_effect.h"
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<MinimumEffect> s_reg("Minimum", "Channel");

MinimumEffect::MinimumEffect() {
    addParameter(EffectParameter::makeInt("radius", "Radius", 1, 100, 5));
}

std::unique_ptr<Effect> MinimumEffect::clone() const {
    auto e = std::make_unique<MinimumEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void MinimumEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    int radius = getIntParam("radius");

    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t best[4] = {255, 255, 255, 255};
            for (int dy = -radius; dy <= radius; dy++) {
                for (int dx = -radius; dx <= radius; dx++) {
                    int sx = std::clamp(x + dx, 0, buffer.width - 1);
                    int sy = std::clamp(y + dy, 0, buffer.height - 1);
                    const uint8_t* p = buffer.pixelAt(sx, sy);
                    for (int c = 0; c < 4; c++) best[c] = std::min(best[c], p[c]);
                }
            }
            uint8_t* dst = tmp.pixelAt(x, y);
            dst[0] = best[0]; dst[1] = best[1]; dst[2] = best[2]; dst[3] = best[3];
        }
    }
    buffer.data = tmp.data;
}

} // namespace FreeEffect
