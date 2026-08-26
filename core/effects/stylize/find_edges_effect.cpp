#include "../effect_registry.h"
#include "find_edges_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<FindEdgesEffect> s_reg("Find Edges", "Stylize");

FindEdgesEffect::FindEdgesEffect() {
    addParameter(EffectParameter::makeBool("invert", "Invert", false));
}

std::vector<ParameterGroup> FindEdgesEffect::getParameterGroups() const {
    return {{getName(), {EffectParameter::makeBool("invert", "Invert", false)}}};
}

std::unique_ptr<Effect> FindEdgesEffect::clone() const {
    auto e = std::make_unique<FindEdgesEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void FindEdgesEffect::render(PixelBuffer& buffer, double time) {
    bool invert = getBoolParam("invert");
    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);

    for (int y = 1; y < buffer.height - 1; y++) {
        for (int x = 1; x < buffer.width - 1; x++) {
            float gx = 0, gy = 0;
            static const int kx[3][3] = {{-1,0,1},{-2,0,2},{-1,0,1}};
            static const int ky[3][3] = {{-1,-2,-1},{0,0,0},{1,2,1}};
            for (int ky2 = -1; ky2 <= 1; ky2++) {
                for (int kx2 = -1; kx2 <= 1; kx2++) {
                    const uint8_t* p = buffer.pixelAt(x + kx2, y + ky2);
                    float lum = p[0] * 0.299f + p[1] * 0.587f + p[2] * 0.114f;
                    gx += lum * kx[ky2 + 1][kx2 + 1];
                    gy += lum * ky[ky2 + 1][kx2 + 1];
                }
            }
            float edge = std::min(std::sqrt(gx * gx + gy * gy), 255.0f);
            uint8_t* dst = tmp.pixelAt(x, y);
            if (invert) {
                edge = 255.0f - edge;
            }
            dst[0] = static_cast<uint8_t>(edge);
            dst[1] = static_cast<uint8_t>(edge);
            dst[2] = static_cast<uint8_t>(edge);
            dst[3] = 255;
        }
    }
    buffer.data = tmp.data;
}

} // namespace FreeEffect
