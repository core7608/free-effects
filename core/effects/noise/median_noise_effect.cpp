#include "../effect_registry.h"
#include "median_noise_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<MedianNoiseEffect> s_reg("Median", "Noise & Grain");

MedianNoiseEffect::MedianNoiseEffect() {
    addParameter(EffectParameter::makeInt("radius", "Radius", 1, 100, 2));
}

std::unique_ptr<Effect> MedianNoiseEffect::clone() const {
    auto e = std::make_unique<MedianNoiseEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void MedianNoiseEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    int radius = getIntParam("radius");
    if (radius <= 0) return;

    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);
    std::copy(buffer.data.begin(), buffer.data.end(), tmp.data.begin());

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            std::vector<uint8_t> rVals, gVals, bVals;
            for (int dy = -radius; dy <= radius; dy++) {
                for (int dx = -radius; dx <= radius; dx++) {
                    int sx = std::clamp(x + dx, 0, buffer.width - 1);
                    int sy = std::clamp(y + dy, 0, buffer.height - 1);
                    const uint8_t* p = tmp.pixelAt(sx, sy);
                    rVals.push_back(p[0]); gVals.push_back(p[1]); bVals.push_back(p[2]);
                }
            }
            std::sort(rVals.begin(), rVals.end());
            std::sort(gVals.begin(), gVals.end());
            std::sort(bVals.begin(), bVals.end());
            int mid = rVals.size() / 2;
            uint8_t* dst = buffer.pixelAt(x, y);
            dst[0] = rVals[mid]; dst[1] = gVals[mid]; dst[2] = bVals[mid];
        }
    }
}

} // namespace FreeEffect
