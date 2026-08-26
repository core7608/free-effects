#include "../effect_registry.h"
#include "cc_power_pin_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<CCPowerPinEffect> s_reg("CC Power Pin", "Distort");

CCPowerPinEffect::CCPowerPinEffect() {
    addParameter(EffectParameter::makeVec2("topLeft", "Top Left", {0.0, 0.0}));
    addParameter(EffectParameter::makeVec2("topRight", "Top Right", {1.0, 0.0}));
    addParameter(EffectParameter::makeVec2("bottomRight", "Bottom Right", {1.0, 1.0}));
    addParameter(EffectParameter::makeVec2("bottomLeft", "Bottom Left", {0.0, 1.0}));
    addParameter(EffectParameter::makeBool("perspective", "Perspective", true));
}

std::unique_ptr<Effect> CCPowerPinEffect::clone() const {
    auto e = std::make_unique<CCPowerPinEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void CCPowerPinEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    Vec2 tl = getVec2Param("topLeft"), tr = getVec2Param("topRight");
    Vec2 br = getVec2Param("bottomRight"), bl = getVec2Param("bottomLeft");

    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);
    std::copy(buffer.data.begin(), buffer.data.end(), tmp.data.begin());

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            float u = static_cast<float>(x) / buffer.width;
            float v = static_cast<float>(y) / buffer.height;

            float topX = tl.x + (tr.x - tl.x) * u;
            float topY = tl.y + (tr.y - tl.y) * u;
            float botX = bl.x + (br.x - bl.x) * u;
            float botY = bl.y + (br.y - bl.y) * u;

            float srcX = (topX + (botX - topX) * v) * buffer.width;
            float srcY = (topY + (botY - topY) * v) * buffer.height;

            int isx = std::clamp(static_cast<int>(srcX + 0.5f), 0, buffer.width - 1);
            int isy = std::clamp(static_cast<int>(srcY + 0.5f), 0, buffer.height - 1);
            const uint8_t* p = tmp.pixelAt(isx, isy);
            uint8_t* dst = buffer.pixelAt(x, y);
            dst[0] = p[0]; dst[1] = p[1]; dst[2] = p[2]; dst[3] = p[3];
        }
    }
}

} // namespace FreeEffect
