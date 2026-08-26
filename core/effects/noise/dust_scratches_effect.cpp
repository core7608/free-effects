#include "../effect_registry.h"
#include "dust_scratches_effect.h"
#include <algorithm>
#include <vector>

namespace FreeEffect {

static EffectRegistrar<DustScratchesEffect> s_reg("Dust & Scratches", "Noise & Grain");

DustScratchesEffect::DustScratchesEffect() {
    addParameter(EffectParameter::makeInt("radius", "Radius", 1, 100, 5));
    addParameter(EffectParameter::makeInt("threshold", "Threshold", 0, 255, 0));
}

std::unique_ptr<Effect> DustScratchesEffect::clone() const {
    auto e = std::make_unique<DustScratchesEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void DustScratchesEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    int radius = getIntParam("radius");
    int threshold = getIntParam("threshold");

    if (radius <= 0) return;

    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            std::vector<uint8_t> vals[4];
            for (int dy = -radius; dy <= radius; dy++) {
                for (int dx = -radius; dx <= radius; dx++) {
                    int sx = std::clamp(x + dx, 0, buffer.width - 1);
                    int sy = std::clamp(y + dy, 0, buffer.height - 1);
                    const uint8_t* p = buffer.pixelAt(sx, sy);
                    vals[0].push_back(p[0]);
                    vals[1].push_back(p[1]);
                    vals[2].push_back(p[2]);
                    vals[3].push_back(p[3]);
                }
            }

            for (int c = 0; c < 4; c++) {
                std::sort(vals[c].begin(), vals[c].end());
                uint8_t median = vals[c][vals[c].size() / 2];
                uint8_t center = buffer.pixelAt(x, y)[c];
                if (threshold == 0 || std::abs(center - median) > threshold) {
                    tmp.pixelAt(x, y)[c] = median;
                } else {
                    tmp.pixelAt(x, y)[c] = center;
                }
            }
        }
    }
    buffer.data = tmp.data;
}

} // namespace FreeEffect
