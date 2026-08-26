#include "../effect_registry.h"
#include "cc_light_sweep_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<CCLightSweepEffect> s_reg("CC Light Sweep", "Generate");

CCLightSweepEffect::CCLightSweepEffect() {
    addParameter(EffectParameter::makeVec2("center", "Center", {0.5, 0.5}));
    addParameter(EffectParameter::makeAngle("direction", "Direction", -30.0));
    addParameter(EffectParameter::makeFloat("width", "Width", 10.0, 500.0, 100.0));
    addParameter(EffectParameter::makeFloat("intensity", "Intensity", 0.0, 200.0, 50.0));
    addParameter(EffectParameter::makeFloat("edgeIntensity", "Edge Intensity", 0.0, 100.0, 50.0));
}

std::unique_ptr<Effect> CCLightSweepEffect::clone() const {
    auto e = std::make_unique<CCLightSweepEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void CCLightSweepEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    Vec2 center = getVec2Param("center");
    float dir = getFloatParam("direction") * 3.14159265f / 180.0f;
    float width = getFloatParam("width");
    float intensity = getFloatParam("intensity") / 100.0f;
    float edgeInt = getFloatParam("edgeIntensity") / 100.0f;

    float cx = center.x * buffer.width, cy = center.y * buffer.height;
    float nx = std::cos(dir), ny = std::sin(dir);
    float maxDist = std::sqrt(buffer.width * buffer.width + buffer.height * buffer.height) * 0.5f;

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            float dx = x - cx, dy = y - cy;
            float dist = dx * nx + dy * ny;
            float sweep = std::abs(dist);
            float edge = std::abs(sweep - width * 0.5f);
            float amount = 0;
            if (sweep < width * 0.5f) amount = intensity * (1.0f - sweep / (width * 0.5f));
            if (edge < width * 0.1f) amount += edgeInt * (1.0f - edge / (width * 0.1f));
            if (amount > 0.01f) {
                uint8_t* p = buffer.pixelAt(x, y);
                p[0] = static_cast<uint8_t>(std::min(static_cast<double>(p[0] + 255.0f * amount), 255.0));
                p[1] = static_cast<uint8_t>(std::min(static_cast<double>(p[1] + 255.0f * amount), 255.0));
                p[2] = static_cast<uint8_t>(std::min(static_cast<double>(p[2] + 255.0f * amount), 255.0));
            }
        }
    }
}

} // namespace FreeEffect
