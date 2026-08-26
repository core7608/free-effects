#include "../effect_registry.h"
#include "pixel_dissolve_effect.h"
#include <cmath>
#include <vector>
#include <algorithm>
#include <random>

namespace FreeEffect {

static EffectRegistrar<PixelDissolveEffect> s_reg("Pixel Dissolve", "Simulation");

PixelDissolveEffect::PixelDissolveEffect() {
    addParameter(EffectParameter::makeFloat("progress", "Progress", 0.0, 1.0, 0.0));
    addParameter(EffectParameter::makeInt("grid_size", "Grid Size", 1, 50, 8));
    addParameter(EffectParameter::makeColor("dissolve_color", "Dissolve Color", Color{0.0, 0.0, 0.0, 0.0}));
    addParameter(EffectParameter::makeBool("soft_edges", "Soft Edges", true));
}

std::vector<ParameterGroup> PixelDissolveEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeFloat("progress", "Progress", 0.0, 1.0, 0.0),
        EffectParameter::makeInt("grid_size", "Grid Size", 1, 50, 8),
        EffectParameter::makeColor("dissolve_color", "Dissolve Color", Color{0.0, 0.0, 0.0, 0.0}),
        EffectParameter::makeBool("soft_edges", "Soft Edges", true)
    }}};
}

std::unique_ptr<Effect> PixelDissolveEffect::clone() const {
    auto e = std::make_unique<PixelDissolveEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void PixelDissolveEffect::render(PixelBuffer& buffer, double time) {
    if (buffer.width == 0 || buffer.height == 0) return;
    double progress = getFloatParam("progress");
    int gridSize = getIntParam("grid_size");
    Color dc = getColorParam("dissolve_color");
    bool soft = getBoolParam("soft_edges");
    if (progress < 0.001) return;
    int gw = (buffer.width + gridSize - 1) / gridSize;
    int gh = (buffer.height + gridSize - 1) / gridSize;
    std::vector<float> thresholds(gw * gh);
    for (int gy = 0; gy < gh; gy++) {
        for (int gx = 0; gx < gw; gx++) {
            int idx = gy * gw + gx;
            double seed = std::sin(gx * 127.1 + gy * 311.7) * 43758.5453;
            thresholds[idx] = static_cast<float>(seed - std::floor(seed));
        }
    }
    double dr = dc.r * 255.0;
    double dg = dc.g * 255.0;
    double db = dc.b * 255.0;
    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            int gx = x / gridSize;
            int gy = y / gridSize;
            if (gx >= gw) gx = gw - 1;
            if (gy >= gh) gy = gh - 1;
            int gIdx = gy * gw + gx;
            float threshold = thresholds[gIdx];
            double dissolveAmount = 0.0;
            if (soft) {
                dissolveAmount = std::clamp((progress - threshold * 0.8) * 3.0, 0.0, 1.0);
            } else {
                dissolveAmount = progress > threshold ? 1.0 : 0.0;
            }
            if (dissolveAmount < 0.01) continue;
            uint8_t* p = buffer.pixelAt(x, y);
            double sa = p[3] / 255.0;
            double da = dissolveAmount;
            double outA = sa * (1.0 - da);
            p[0] = static_cast<uint8_t>(p[0] * (1.0 - da) + dr * da);
            p[1] = static_cast<uint8_t>(p[1] * (1.0 - da) + dg * da);
            p[2] = static_cast<uint8_t>(p[2] * (1.0 - da) + db * da);
            p[3] = static_cast<uint8_t>(std::clamp(outA * 255.0, 0.0, 255.0));
        }
    }
}

} // namespace FreeEffect
