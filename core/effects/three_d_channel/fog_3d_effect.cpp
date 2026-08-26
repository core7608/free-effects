#include "../effect_registry.h"
#include "fog_3d_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<Fog3DEffect> s_reg("Fog 3D", "3D Channel");

Fog3DEffect::Fog3DEffect() {
    addParameter(EffectParameter::makeFloat("fogStartDepth", "Fog Start Depth", 0.0, 10000.0, 0.0));
    addParameter(EffectParameter::makeFloat("fogEndDepth", "Fog End Depth", 0.0, 10000.0, 5000.0));
    addParameter(EffectParameter::makeColor("fogColor", "Fog Color", {200.0, 200.0, 200.0, 1.0}));
    addParameter(EffectParameter::makeFloat("fogDensity", "Fog Density", 0.0, 100.0, 10.0));
}

std::unique_ptr<Effect> Fog3DEffect::clone() const {
    auto e = std::make_unique<Fog3DEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void Fog3DEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    float fogStart = getFloatParam("fogStartDepth");
    float fogEnd = getFloatParam("fogEndDepth");
    Color fogCol = getColorParam("fogColor");
    float density = getFloatParam("fogDensity") / 100.0f;

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            float simDepth = static_cast<float>(y) / buffer.height * 10000.0f;
            float fogFactor = std::clamp((simDepth - fogStart) / (fogEnd - fogStart + 0.001f), 0.0f, 1.0f);
            fogFactor = fogFactor * density;
            p[0] = static_cast<uint8_t>(std::clamp(static_cast<double>(p[0] * (1.0f - fogFactor) + fogCol.r * fogFactor), 0.0, 255.0));
            p[1] = static_cast<uint8_t>(std::clamp(static_cast<double>(p[1] * (1.0f - fogFactor) + fogCol.g * fogFactor), 0.0, 255.0));
            p[2] = static_cast<uint8_t>(std::clamp(static_cast<double>(p[2] * (1.0f - fogFactor) + fogCol.b * fogFactor), 0.0, 255.0));
        }
    }
}

} // namespace FreeEffect
