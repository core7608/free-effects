#include "../effect_registry.h"
#include "morph_transition.h"
#include <cmath>
#include <vector>
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<MorphTransition> s_reg("Morph Transition", "Transition");

MorphTransition::MorphTransition() {
    addParameter(EffectParameter::makeFloat("progress", "Progress", 0.0, 1.0, 0.0));
    addParameter(EffectParameter::makeInt("grid_size", "Grid Size", 2, 50, 10));
    addParameter(EffectParameter::makeFloat("warp", "Warp Amount", 0.0, 1.0, 0.5));
}

std::vector<ParameterGroup> MorphTransition::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeFloat("progress", "Progress", 0.0, 1.0, 0.0),
        EffectParameter::makeInt("grid_size", "Grid Size", 2, 50, 10),
        EffectParameter::makeFloat("warp", "Warp Amount", 0.0, 1.0, 0.5)
    }}};
}

std::unique_ptr<Effect> MorphTransition::clone() const {
    auto e = std::make_unique<MorphTransition>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void MorphTransition::render(PixelBuffer& buffer, double time) {
    if (buffer.width == 0 || buffer.height == 0) return;
    double progress = getFloatParam("progress");
    int gridSize = getIntParam("grid_size");
    double warpAmt = getFloatParam("warp");
    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);
    tmp.data = buffer.data;
    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            double gx = static_cast<double>(x / gridSize);
            double gy = static_cast<double>(y / gridSize);
            double seed1 = std::sin(gx * 127.1 + gy * 311.7) * 43758.5453;
            double seed2 = std::sin(gx * 269.5 + gy * 183.3) * 43758.5453;
            double f1 = seed1 - std::floor(seed1);
            double f2 = seed2 - std::floor(seed2);
            double warpX = (f1 - 0.5) * warpAmt * gridSize * 2.0 * progress * (1.0 - progress) * 4.0;
            double warpY = (f2 - 0.5) * warpAmt * gridSize * 2.0 * progress * (1.0 - progress) * 4.0;
            int sx = std::clamp(static_cast<int>(x + warpX), 0, buffer.width - 1);
            int sy = std::clamp(static_cast<int>(y + warpY), 0, buffer.height - 1);
            const uint8_t* src = tmp.pixelAt(sx, sy);
            uint8_t* dst = buffer.pixelAt(x, y);
            double fade = std::abs(std::sin(progress * 3.14159));
            dst[0] = static_cast<uint8_t>(src[0] * (1.0 - fade * 0.3));
            dst[1] = static_cast<uint8_t>(src[1] * (1.0 - fade * 0.3));
            dst[2] = static_cast<uint8_t>(src[2] * (1.0 - fade * 0.3));
            dst[3] = static_cast<uint8_t>(src[3]);
        }
    }
}

} // namespace FreeEffect
