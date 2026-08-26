#include "../effect_registry.h"
#include "cc_prism_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<CCPrismEffect> s_reg("CC Prism", "Stylize");

CCPrismEffect::CCPrismEffect() {
    addParameter(EffectParameter::makeVec2("center", "Center", {0.5, 0.5}));
    addParameter(EffectParameter::makeFloat("amount", "Amount", 0.0, 100.0, 10.0));
    addParameter(EffectParameter::makeAngle("direction", "Direction", 0.0));
}

std::unique_ptr<Effect> CCPrismEffect::clone() const {
    auto e = std::make_unique<CCPrismEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void CCPrismEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    Vec2 center = getVec2Param("center");
    float amount = getFloatParam("amount");
    float dir = getFloatParam("direction") * 3.14159265f / 180.0f;

    float cx = center.x * buffer.width, cy = center.y * buffer.height;
    float cosD = std::cos(dir), sinD = std::sin(dir);

    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);
    std::copy(buffer.data.begin(), buffer.data.end(), tmp.data.begin());

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            float dx = x - cx, dy = y - cy;
            float dist = std::sqrt(dx * dx + dy * dy);
            float offset = dist * amount * 0.01f;

            uint8_t* p = buffer.pixelAt(x, y);
            int rx = std::clamp(static_cast<int>(x + cosD * offset * 2), 0, buffer.width - 1);
            int ry = std::clamp(static_cast<int>(y + sinD * offset * 2), 0, buffer.height - 1);
            int gx = std::clamp(static_cast<int>(x - cosD * offset), 0, buffer.width - 1);
            int gy = std::clamp(static_cast<int>(y - sinD * offset), 0, buffer.height - 1);
            int bx = std::clamp(static_cast<int>(x + cosD * offset), 0, buffer.width - 1);
            int by = std::clamp(static_cast<int>(y + sinD * offset), 0, buffer.height - 1);
            p[0] = tmp.pixelAt(rx, ry)[0];
            p[1] = tmp.pixelAt(gx, gy)[1];
            p[2] = tmp.pixelAt(bx, by)[2];
        }
    }
}

} // namespace FreeEffect
