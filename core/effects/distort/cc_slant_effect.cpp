#include "../effect_registry.h"
#include "cc_slant_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<CCSlantEffect> s_reg("CC Slant", "Distort");

CCSlantEffect::CCSlantEffect() {
    addParameter(EffectParameter::makeFloat("slant", "Slant", -100.0, 100.0, 0.0));
    addParameter(EffectParameter::makeFloat("height", "Height", 0.0, 100.0, 100.0));
    addParameter(EffectParameter::makeVec2("floor", "Floor", {0.5, 0.5}));
    addParameter(EffectParameter::makeBool("floorMode", "Floor Is On", true));
}

std::unique_ptr<Effect> CCSlantEffect::clone() const {
    auto e = std::make_unique<CCSlantEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void CCSlantEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    float slant = getFloatParam("slant") / 100.0f;
    float height = getFloatParam("height") / 100.0f;
    Vec2 floorPos = getVec2Param("floor");
    bool floorOn = getBoolParam("floorMode");

    float floorY = floorPos.y * buffer.height;
    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);
    std::copy(buffer.data.begin(), buffer.data.end(), tmp.data.begin());

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            float dy = (y - floorY) / (buffer.height * height + 1.0f);
            float offsetX = slant * dy * buffer.width * 0.5f;
            float srcX = x + offsetX;
            float srcY = y;
            if (floorOn) srcY = floorY + (y - floorY) / std::max(height, 0.01f);

            int isx = std::clamp(static_cast<int>(srcX + 0.5f), 0, buffer.width - 1);
            int isy = std::clamp(static_cast<int>(srcY + 0.5f), 0, buffer.height - 1);
            const uint8_t* p = tmp.pixelAt(isx, isy);
            uint8_t* dst = buffer.pixelAt(x, y);
            dst[0] = p[0]; dst[1] = p[1]; dst[2] = p[2]; dst[3] = p[3];
        }
    }
}

} // namespace FreeEffect
