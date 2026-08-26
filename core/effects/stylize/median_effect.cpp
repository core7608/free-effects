#include "../effect_registry.h"
#include "median_effect.h"
#include <cmath>
#include <vector>
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<MedianEffect> s_reg("Median", "Stylize");

MedianEffect::MedianEffect() {
    addParameter(EffectParameter::makeInt("radius", "Radius", 1, 20, 3));
}

std::vector<ParameterGroup> MedianEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeInt("radius", "Radius", 1, 20, 3)
    }}};
}

std::unique_ptr<Effect> MedianEffect::clone() const {
    auto e = std::make_unique<MedianEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void MedianEffect::render(PixelBuffer& buffer, double time) {
    if (buffer.width == 0 || buffer.height == 0) return;
    int radius = getIntParam("radius");
    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);
    tmp.data = buffer.data;
    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            std::vector<uint8_t> vals[4];
            for (int dy = -radius; dy <= radius; dy++) {
                for (int dx = -radius; dx <= radius; dx++) {
                    int sx = std::clamp(x + dx, 0, buffer.width - 1);
                    int sy = std::clamp(y + dy, 0, buffer.height - 1);
                    const uint8_t* p = tmp.pixelAt(sx, sy);
                    for (int c = 0; c < 4; c++) vals[c].push_back(p[c]);
                }
            }
            uint8_t* p = buffer.pixelAt(x, y);
            for (int c = 0; c < 4; c++) {
                std::sort(vals[c].begin(), vals[c].end());
                p[c] = vals[c][vals[c].size() / 2];
            }
        }
    }
}

} // namespace FreeEffect
