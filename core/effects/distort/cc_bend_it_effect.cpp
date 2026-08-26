#include "../effect_registry.h"
#include "cc_bend_it_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<CCBendItEffect> s_reg("CC Bend It", "Distort");

CCBendItEffect::CCBendItEffect() {
    addParameter(EffectParameter::makeVec2("start", "Start", {0.0, 0.5}));
    addParameter(EffectParameter::makeVec2("end", "End", {1.0, 0.5}));
    addParameter(EffectParameter::makeFloat("bend", "Bend", -200.0, 200.0, 0.0));
    addParameter(EffectParameter::makeFloat("renderStart", "Render Start", 0.0, 100.0, 0.0));
    addParameter(EffectParameter::makeFloat("renderEnd", "Render End", 0.0, 100.0, 100.0));
}

std::unique_ptr<Effect> CCBendItEffect::clone() const {
    auto e = std::make_unique<CCBendItEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void CCBendItEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    Vec2 start = getVec2Param("start");
    Vec2 end = getVec2Param("end");
    float bend = getFloatParam("bend") / 100.0f;

    float sx = start.x * buffer.width, sy = start.y * buffer.height;
    float ex = end.x * buffer.width, ey = end.y * buffer.height;
    float dx = ex - sx, dy = ey - sy;
    float lineLen = std::sqrt(dx * dx + dy * dy);
    if (lineLen < 1.0f) return;
    float nx = dx / lineLen, ny = dy / lineLen;
    float px = -ny, py = nx;

    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);
    std::copy(buffer.data.begin(), buffer.data.end(), tmp.data.begin());

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            float fx = x - sx, fy = y - sy;
            float t = (fx * dx + fy * dy) / (lineLen * lineLen);
            float s = (fx * px + fy * py);
            float curvature = bend * t * t;
            float newT = t;
            float newS = s - curvature * s;
            float srcX = sx + newT * dx + newS * px;
            float srcY = sy + newT * dy + newS * py;
            int isx = std::clamp(static_cast<int>(srcX + 0.5f), 0, buffer.width - 1);
            int isy = std::clamp(static_cast<int>(srcY + 0.5f), 0, buffer.height - 1);
            const uint8_t* p = tmp.pixelAt(isx, isy);
            uint8_t* dst = buffer.pixelAt(x, y);
            dst[0] = p[0]; dst[1] = p[1]; dst[2] = p[2]; dst[3] = p[3];
        }
    }
}

} // namespace FreeEffect
