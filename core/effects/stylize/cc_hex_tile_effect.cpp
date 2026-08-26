#include "../effect_registry.h"
#include "cc_hex_tile_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<CCHexTileEffect> s_reg("CC HexTile", "Stylize");

CCHexTileEffect::CCHexTileEffect() {
    addParameter(EffectParameter::makeVec2("center", "Center", {0.5, 0.5}));
    addParameter(EffectParameter::makeFloat("size", "Size", 5.0, 200.0, 50.0));
    addParameter(EffectParameter::makeAngle("rotation", "Rotation", 0.0));
}

std::unique_ptr<Effect> CCHexTileEffect::clone() const {
    auto e = std::make_unique<CCHexTileEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void CCHexTileEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    Vec2 center = getVec2Param("center");
    float size = getFloatParam("size");
    float rot = getFloatParam("rotation") * 3.14159265f / 180.0f;

    float cx = center.x * buffer.width, cy = center.y * buffer.height;
    float hexH = size * std::sqrt(3.0f);
    float hexW = size * 2.0f;

    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);
    std::copy(buffer.data.begin(), buffer.data.end(), tmp.data.begin());

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            float dx = x - cx, dy = y - cy;
            float rx = dx * std::cos(rot) + dy * std::sin(rot);
            float ry = -dx * std::sin(rot) + dy * std::cos(rot);
            int col = static_cast<int>(std::round(rx / (hexW * 0.75f)));
            float rowOff = (col % 2 != 0) ? hexH * 0.5f : 0.0f;
            int row = static_cast<int>(std::round((ry - rowOff) / hexH));

            float hexCX = col * hexW * 0.75f;
            float hexCY = row * hexH + rowOff;
            float srcX = cx + hexCX * std::cos(rot) - hexCY * std::sin(rot);
            float srcY = cy + hexCX * std::sin(rot) + hexCY * std::cos(rot);
            int isx = std::clamp(static_cast<int>(srcX), 0, buffer.width - 1);
            int isy = std::clamp(static_cast<int>(srcY), 0, buffer.height - 1);
            const uint8_t* p = tmp.pixelAt(isx, isy);
            uint8_t* dst = buffer.pixelAt(x, y);
            dst[0] = p[0]; dst[1] = p[1]; dst[2] = p[2]; dst[3] = p[3];
        }
    }
}

} // namespace FreeEffect
