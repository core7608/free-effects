#include "../effect_registry.h"
#include "cc_bender_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<CCBenderEffect> s_reg("CC Bender", "Distort");

CCBenderEffect::CCBenderEffect() {
    addParameter(EffectParameter::makeVec2("anchor", "Anchor", {0.5, 0.5}));
    addParameter(EffectParameter::makeFloat("amount", "Amount", -200.0, 200.0, 0.0));
    addParameter(EffectParameter::makeDropdown("axis", "Axis", {"X", "Y", "XY"}, 2));
}

std::unique_ptr<Effect> CCBenderEffect::clone() const {
    auto e = std::make_unique<CCBenderEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void CCBenderEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    Vec2 anchor = getVec2Param("anchor");
    float amount = getFloatParam("amount") / 100.0f;
    int axis = getDropdownParam("axis");

    float ax = anchor.x * buffer.width, ay = anchor.y * buffer.height;
    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);
    std::copy(buffer.data.begin(), buffer.data.end(), tmp.data.begin());

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            float dx = x - ax, dy = y - ay;
            float srcX = x, srcY = y;

            if (axis != 1) {
                float t = dx / (buffer.width * 0.5f);
                srcY += amount * dy * t * t * 0.5f;
            }
            if (axis != 0) {
                float t = dy / (buffer.height * 0.5f);
                srcX += amount * dx * t * t * 0.5f;
            }

            int isx = std::clamp(static_cast<int>(srcX + 0.5f), 0, buffer.width - 1);
            int isy = std::clamp(static_cast<int>(srcY + 0.5f), 0, buffer.height - 1);
            const uint8_t* p = tmp.pixelAt(isx, isy);
            uint8_t* dst = buffer.pixelAt(x, y);
            dst[0] = p[0]; dst[1] = p[1]; dst[2] = p[2]; dst[3] = p[3];
        }
    }
}

} // namespace FreeEffect
