#include "../effect_registry.h"
#include "tile_effect.h"
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<TileEffect> s_reg("Tile", "Utility");

TileEffect::TileEffect() {
    addParameter(EffectParameter::makeInt("width", "Output Width", 1, 4096, 100));
    addParameter(EffectParameter::makeInt("height", "Output Height", 1, 4096, 100));
    addParameter(EffectParameter::makeVec2("offset", "Offset", {0.0, 0.0}));
    addParameter(EffectParameter::makeBool("mirrorEdges", "Mirror Edges", false));
}

std::vector<ParameterGroup> TileEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeInt("width", "Output Width", 1, 4096, false),
        EffectParameter::makeInt("height", "Output Height", 1, 4096, false),
        EffectParameter::makeVec2("offset", "Offset", {0.0, 0.0}),
        EffectParameter::makeBool("mirrorEdges", "Mirror Edges", false)
    }}};
}

std::unique_ptr<Effect> TileEffect::clone() const {
    auto e = std::make_unique<TileEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void TileEffect::render(PixelBuffer& buffer, double time) {
    int outW = std::max(getIntParam("width"), 1);
    int outH = std::max(getIntParam("height"), 1);
    bool mirror = getBoolParam("mirrorEdges");
    Vec2 offset = getVec2Param("offset");

    int offX = static_cast<int>(offset.x * buffer.width);
    int offY = static_cast<int>(offset.y * buffer.height);

    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            int tx = (x + offX) % buffer.width;
            int ty = (y + offY) % buffer.height;
            if (tx < 0) tx += buffer.width;
            if (ty < 0) ty += buffer.height;
            if (mirror) {
                int gx = (x + offX) / buffer.width;
                int gy = (y + offY) / buffer.height;
                if (((gx % 2) + 2) % 2 == 1) tx = buffer.width - 1 - ((x + offX) % buffer.width);
                if (((gy % 2) + 2) % 2 == 1) ty = buffer.height - 1 - ((y + offY) % buffer.height);
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
