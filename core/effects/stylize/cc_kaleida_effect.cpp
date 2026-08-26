#include "../effect_registry.h"
#include "cc_kaleida_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<CCKaleidaEffect> s_reg("CC Kaleida", "Stylize");

CCKaleidaEffect::CCKaleidaEffect() {
    addParameter(EffectParameter::makeVec2("center", "Center", {0.5, 0.5}));
    addParameter(EffectParameter::makeFloat("size", "Size", 1.0, 500.0, 100.0));
    addParameter(EffectParameter::makeAngle("rotation", "Rotation", 0.0));
    addParameter(EffectParameter::makeDropdown("mode", "Mirroring", {"Normal", "Star", "Wheel", "Translate"}, 0));
}

std::unique_ptr<Effect> CCKaleidaEffect::clone() const {
    auto e = std::make_unique<CCKaleidaEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void CCKaleidaEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    Vec2 center = getVec2Param("center");
    float size = getFloatParam("size");
    float rot = getFloatParam("rotation") * 3.14159265f / 180.0f;
    int mode = getDropdownParam("mode");

    float cx = center.x * buffer.width, cy = center.y * buffer.height;
    float segments = (mode == 1) ? 6.0f : (mode == 2) ? 8.0f : 4.0f;

    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);
    std::copy(buffer.data.begin(), buffer.data.end(), tmp.data.begin());

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            float dx = x - cx, dy = y - cy;
            float angle = std::atan2(dy, dx) + rot;
            float dist = std::sqrt(dx * dx + dy * dy);
            float segAngle = 2.0f * 3.14159265f / segments;
            float seg = std::fmod(angle, segAngle);
            if (seg < 0) seg += segAngle;
            if (seg > segAngle * 0.5f) seg = segAngle - seg;

            float srcX = cx + std::cos(seg) * dist;
            float srcY = cy + std::sin(seg) * dist;
            int isx = std::clamp(static_cast<int>(srcX), 0, buffer.width - 1);
            int isy = std::clamp(static_cast<int>(srcY), 0, buffer.height - 1);
            const uint8_t* p = tmp.pixelAt(isx, isy);
            uint8_t* dst = buffer.pixelAt(x, y);
            dst[0] = p[0]; dst[1] = p[1]; dst[2] = p[2]; dst[3] = p[3];
        }
    }
}

} // namespace FreeEffect
