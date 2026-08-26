#include "../effect_registry.h"
#include "cc_light_burst_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<CCLightBurstEffect> s_reg("CC Light Burst", "Generate");

CCLightBurstEffect::CCLightBurstEffect() {
    addParameter(EffectParameter::makeVec2("center", "Center", {0.5, 0.5}));
    addParameter(EffectParameter::makeFloat("intensity", "Intensity", 0.0, 100.0, 50.0));
    addParameter(EffectParameter::makeInt("rayLength", "Ray Length", 0, 500, 100));
    addParameter(EffectParameter::makeFloat("softness", "Softness", 0.0, 100.0, 50.0));
    addParameter(EffectParameter::makeAngle("direction", "Direction", 0.0));
}

std::unique_ptr<Effect> CCLightBurstEffect::clone() const {
    auto e = std::make_unique<CCLightBurstEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void CCLightBurstEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    Vec2 center = getVec2Param("center");
    float intensity = getFloatParam("intensity") / 100.0f;
    int rayLen = getIntParam("rayLength");
    float soft = getFloatParam("softness") / 100.0f;
    float dir = getFloatParam("direction") * 3.14159265f / 180.0f;

    float cx = center.x * buffer.width, cy = center.y * buffer.height;
    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            float dx = x - cx, dy = y - cy;
            float dist = std::sqrt(dx * dx + dy * dy);
            float maxDist = std::sqrt(cx * cx + cy * cy);
            float angle = std::atan2(dy, dx) - dir;
            float amount = std::max(0.0f, 1.0f - dist / (maxDist + 1.0f));
            amount = amount * intensity;
            float samples = 0;
            float rS = 0, gS = 0, bS = 0;
            int numSamples = std::min(rayLen / 4, 20);
            for (int s = 0; s < numSamples; s++) {
                float t = static_cast<float>(s) / numSamples;
                float sx2 = cx + dx * t * 2.0f;
                float sy2 = cy + dy * t * 2.0f;
                int isx = std::clamp(static_cast<int>(sx2), 0, buffer.width - 1);
                int isy = std::clamp(static_cast<int>(sy2), 0, buffer.height - 1);
                const uint8_t* p = buffer.pixelAt(isx, isy);
                float w = (1.0f - t) * (1.0f + soft);
                rS += p[0] * w; gS += p[1] * w; bS += p[2] * w;
                samples += w;
            }
            if (samples > 0 && amount > 0.01f) {
                uint8_t* p = buffer.pixelAt(x, y);
                p[0] = static_cast<uint8_t>(std::clamp(static_cast<double>(p[0] + rS / samples * amount), 0.0, 255.0));
                p[1] = static_cast<uint8_t>(std::clamp(static_cast<double>(p[1] + gS / samples * amount), 0.0, 255.0));
                p[2] = static_cast<uint8_t>(std::clamp(static_cast<double>(p[2] + bS / samples * amount), 0.0, 255.0));
            }
        }
    }
}

} // namespace FreeEffect
