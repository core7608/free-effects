#include "../effect_registry.h"
#include "block_dissolve_effect.h"
#include <cmath>
#include <vector>
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<BlockDissolveEffect> s_reg("Block Dissolve", "Transition");

BlockDissolveEffect::BlockDissolveEffect() {
    addParameter(EffectParameter::makeFloat("progress", "Progress", 0.0, 1.0, 0.0));
    addParameter(EffectParameter::makeInt("block_size", "Block Size", 2, 100, 20));
    addParameter(EffectParameter::makeFloat("randomness", "Randomness", 0.0, 1.0, 0.5));
}

std::vector<ParameterGroup> BlockDissolveEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeFloat("progress", "Progress", 0.0, 1.0, 0.0),
        EffectParameter::makeInt("block_size", "Block Size", 2, 100, 20),
        EffectParameter::makeFloat("randomness", "Randomness", 0.0, 1.0, 0.5)
    }}};
}

std::unique_ptr<Effect> BlockDissolveEffect::clone() const {
    auto e = std::make_unique<BlockDissolveEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void BlockDissolveEffect::render(PixelBuffer& buffer, double time) {
    if (buffer.width == 0 || buffer.height == 0) return;
    double progress = getFloatParam("progress");
    int blockSize = getIntParam("block_size");
    double randomness = getFloatParam("randomness");
    int gw = (buffer.width + blockSize - 1) / blockSize;
    int gh = (buffer.height + blockSize - 1) / blockSize;
    std::vector<float> thresholds(gw * gh);
    for (int gy = 0; gy < gh; gy++) {
        for (int gx = 0; gx < gw; gx++) {
            int idx = gy * gw + gx;
            double seed = std::sin(gx * 127.1 + gy * 311.7) * 43758.5453;
            thresholds[idx] = static_cast<float>(seed - std::floor(seed));
        }
    }
    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            int gx = x / blockSize;
            int gy = y / blockSize;
            if (gx >= gw) gx = gw - 1;
            if (gy >= gh) gy = gh - 1;
            int gIdx = gy * gw + gx;
            float threshold = thresholds[gIdx];
            double rowProgress = progress * (1.0 + randomness * (threshold - 0.5));
            double dissolved = rowProgress > threshold ? 1.0 : 0.0;
            if (dissolved < 0.5) continue;
            uint8_t* p = buffer.pixelAt(x, y);
            p[0] = p[1] = p[2] = p[3] = 0;
        }
    }
}

} // namespace FreeEffect
