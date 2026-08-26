#include "../effect_registry.h"
#include "cc_tiler_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<CCTilerEffect> s_reg("CC Tiler", "Distort");

CCTilerEffect::CCTilerEffect() {
    addParameter(EffectParameter::makeVec2("center", "Center", {0.5, 0.5}));
    addParameter(EffectParameter::makeVec2("tiling", "Tiling", {2.0, 2.0}));
    addParameter(EffectParameter::makeAngle("rotation", "Rotation", 0.0));
}

std::unique_ptr<Effect> CCTilerEffect::clone() const {
    auto e = std::make_unique<CCTilerEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void CCTilerEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    Vec2 center = getVec2Param("center");
    Vec2 tiling = getVec2Param("tiling");
    float rot = getFloatParam("rotation") * 3.14159265f / 180.0f;

    float cx = center.x * buffer.width;
    float cy = center.y * buffer.height;
    float tileW = buffer.width / std::max(tiling.x, 0.1);
    float tileH = buffer.height / std::max(tiling.y, 0.1);
    float cosR = std::cos(rot), sinR = std::sin(rot);

    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);
    std::copy(buffer.data.begin(), buffer.data.end(), tmp.data.begin());

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            float dx = x - cx, dy = y - cy;
            float rx = dx * cosR + dy * sinR;
            float ry = -dx * sinR + dy * cosR;
            float srcX = cx + rx;
            float srcY = cy + ry;
            int isx = std::fmod(srcX, tileW);
            int isy = std::fmod(srcY, tileH);
            if (isx < 0) isx += static_cast<int>(tileW);
            if (isy < 0) isy += static_cast<int>(tileH);
            isx = std::clamp(isx, 0, buffer.width - 1);
            isy = std::clamp(isy, 0, buffer.height - 1);
            const uint8_t* p = tmp.pixelAt(isx, isy);
            uint8_t* dst = buffer.pixelAt(x, y);
            dst[0] = p[0]; dst[1] = p[1]; dst[2] = p[2]; dst[3] = p[3];
        }
    }
}

} // namespace FreeEffect
