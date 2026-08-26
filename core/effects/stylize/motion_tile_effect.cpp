#include "../effect_registry.h"
#include "motion_tile_effect.h"
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<MotionTileEffect> s_reg("Motion Tile", "Stylize");

MotionTileEffect::MotionTileEffect() {
    addParameter(EffectParameter::makeInt("outputWidth", "Output Width", 1, 4096, 100));
    addParameter(EffectParameter::makeInt("outputHeight", "Output Height", 1, 4096, 100));
    addParameter(EffectParameter::makeBool("mirrorEdges", "Mirror Edges", false));
    addParameter(EffectParameter::makeVec2("center", "Tile Center", {0.5, 0.5}));
}

std::vector<ParameterGroup> MotionTileEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeInt("outputWidth", "Output Width", 1, 4096, false),
        EffectParameter::makeInt("outputHeight", "Output Height", 1, 4096, false),
        EffectParameter::makeBool("mirrorEdges", "Mirror Edges", false),
        EffectParameter::makeVec2("center", "Tile Center", {0.5, 0.5})
    }}};
}

std::unique_ptr<Effect> MotionTileEffect::clone() const {
    auto e = std::make_unique<MotionTileEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void MotionTileEffect::render(PixelBuffer& buffer, double time) {
    int outW = std::max(getIntParam("outputWidth"), 1);
    int outH = std::max(getIntParam("outputHeight"), 1);
    bool mirror = getBoolParam("mirrorEdges");

    float scaleX = static_cast<float>(buffer.width) / outW * 100.0f;
    float scaleY = static_cast<float>(buffer.height) / outH * 100.0f;

    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            int tx = x % buffer.width;
            int ty = y % buffer.height;
            if (mirror) {
                int gridX = x / buffer.width;
                int gridY = y / buffer.height;
                if (gridX % 2 == 1) tx = buffer.width - 1 - (x % buffer.width);
                if (gridY % 2 == 1) ty = buffer.height - 1 - (y % buffer.height);
            }
            tx = std::clamp(tx, 0, buffer.width - 1);
            ty = std::clamp(ty, 0, buffer.height - 1);
            const uint8_t* src = buffer.pixelAt(tx, ty);
            uint8_t* dst = tmp.pixelAt(x, y);
            dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3];
        }
    }
    buffer.data = tmp.data;
}

} // namespace FreeEffect
