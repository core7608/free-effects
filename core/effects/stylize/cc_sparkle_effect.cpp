#include "../effect_registry.h"
#include "cc_sparkle_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<CCSparkleEffect> s_reg("CC Sparkle", "Stylize");

CCSparkleEffect::CCSparkleEffect() {
    addParameter(EffectParameter::makeVec2("center", "Center", {0.5, 0.5}));
    addParameter(EffectParameter::makeFloat("sparkleLength", "Sparkle Length", 0.0, 100.0, 50.0));
    addParameter(EffectParameter::makeFloat("sparkleWidth", "Sparkle Width", 1.0, 50.0, 2.0));
    addParameter(EffectParameter::makeColor("color", "Color", {255.0, 255.0, 255.0, 1.0}));
    addParameter(EffectParameter::makeFloat("brightness", "Brightness", 0.0, 200.0, 100.0));
    addParameter(EffectParameter::makeAngle("direction", "Direction", 0.0));
}

std::unique_ptr<Effect> CCSparkleEffect::clone() const {
    auto e = std::make_unique<CCSparkleEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void CCSparkleEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    Vec2 center = getVec2Param("center");
    float sLen = getFloatParam("sparkleLength");
    float sWidth = getFloatParam("sparkleWidth");
    Color col = getColorParam("color");
    float bright = getFloatParam("brightness") / 100.0f;
    float dir = getFloatParam("direction") * 3.14159265f / 180.0f;

    float cx = center.x * buffer.width, cy = center.y * buffer.height;
    float cosD = std::cos(dir), sinD = std::sin(dir);

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            float dx = x - cx, dy = y - cy;
            float along = dx * cosD + dy * sinD;
            float across = -dx * sinD + dy * cosD;
            float alongNorm = std::abs(along) / (sLen * buffer.width * 0.01f + 1.0f);
            float acrossNorm = std::abs(across) / (sWidth * 5.0f + 1.0f);
            if (alongNorm < 1.0f && acrossNorm < 1.0f) {
                float sparkle = (1.0f - alongNorm) * (1.0f - acrossNorm) * bright;
                uint8_t* p = buffer.pixelAt(x, y);
                p[0] = static_cast<uint8_t>(std::min(255.0, static_cast<double>(p[0] + col.r * sparkle)));
                p[1] = static_cast<uint8_t>(std::min(255.0, static_cast<double>(p[1] + col.g * sparkle)));
                p[2] = static_cast<uint8_t>(std::min(255.0, static_cast<double>(p[2] + col.b * sparkle)));
            }
        }
    }
}

} // namespace FreeEffect
