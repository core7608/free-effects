#include "../effect_registry.h"
#include "cc_glass_stretch_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<CCGlassStretchEffect> s_reg("CC Glass Stretch", "Stylize");

CCGlassStretchEffect::CCGlassStretchEffect() {
    addParameter(EffectParameter::makeFloat("stretch", "Stretch", 0.0, 100.0, 50.0));
    addParameter(EffectParameter::makeVec2("center", "Center", {0.5, 0.5}));
    addParameter(EffectParameter::makeFloat("radius", "Radius", 0.0, 100.0, 50.0));
}

std::unique_ptr<Effect> CCGlassStretchEffect::clone() const {
    auto e = std::make_unique<CCGlassStretchEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void CCGlassStretchEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    float stretch = getFloatParam("stretch") / 100.0f;
    Vec2 center = getVec2Param("center");
    float radius = getFloatParam("radius") / 100.0f;

    float cx = center.x * buffer.width, cy = center.y * buffer.height;
    float maxR = radius * std::max(buffer.width, buffer.height);

    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);
    std::copy(buffer.data.begin(), buffer.data.end(), tmp.data.begin());

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            float dx = x - cx, dy = y - cy;
            float dist = std::sqrt(dx * dx + dy * dy);
            float factor = std::max(0.0f, 1.0f - dist / (maxR + 1.0f));
            factor = factor * stretch;
            float srcX = cx + dx * (1.0f + factor * 2.0f);
            float srcY = cy + dy * (1.0f + factor * 2.0f);
            int isx = std::clamp(static_cast<int>(srcX), 0, buffer.width - 1);
            int isy = std::clamp(static_cast<int>(srcY), 0, buffer.height - 1);
            const uint8_t* p = tmp.pixelAt(isx, isy);
            uint8_t* dst = buffer.pixelAt(x, y);
            dst[0] = p[0]; dst[1] = p[1]; dst[2] = p[2]; dst[3] = p[3];
        }
    }
}

} // namespace FreeEffect
