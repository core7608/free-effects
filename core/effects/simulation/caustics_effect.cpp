#include "../effect_registry.h"
#include "caustics_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<CausticsEffect> s_reg("Caustics", "Simulation");

CausticsEffect::CausticsEffect() {
    addParameter(EffectParameter::makeFloat("bottom", "Bottom", 0.0, 100.0, 50.0));
    addParameter(EffectParameter::makeFloat("smoothness", "Smoothness", 0.0, 100.0, 50.0));
    addParameter(EffectParameter::makeFloat("waterDepth", "Water Depth", 0.0, 500.0, 200.0));
    addParameter(EffectParameter::makeVec2("lightScale", "Light Scale", {50.0, 50.0}));
}

std::unique_ptr<Effect> CausticsEffect::clone() const {
    auto e = std::make_unique<CausticsEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void CausticsEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    float t = static_cast<float>(time) * 0.5f;
    Vec2 lightScale = getVec2Param("lightScale");
    float smooth = getFloatParam("smoothness") / 100.0f;

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            float nx = static_cast<float>(x) / lightScale.x;
            float ny = static_cast<float>(y) / lightScale.y;
            float caustic = 0;
            caustic += std::sin(nx * 2.0f + t) * std::cos(ny * 2.0f + t * 0.7f);
            caustic += std::sin(nx * 3.7f + ny * 2.3f + t * 1.3f) * 0.5f;
            caustic += std::sin(nx * 5.1f - ny * 4.0f + t * 2.1f) * 0.25f;
            caustic = (caustic + 1.75f) / 3.5f;
            caustic = std::pow(caustic, 1.0f + (1.0f - smooth) * 2.0f);

            uint8_t* p = buffer.pixelAt(x, y);
            p[0] = static_cast<uint8_t>(std::clamp(static_cast<double>(p[0] * caustic), 0.0, 255.0));
            p[1] = static_cast<uint8_t>(std::clamp(static_cast<double>(p[1] * caustic), 0.0, 255.0));
            p[2] = static_cast<uint8_t>(std::clamp(static_cast<double>(p[2] * (caustic * 1.1f)), 0.0, 255.0));
        }
    }
}

} // namespace FreeEffect
