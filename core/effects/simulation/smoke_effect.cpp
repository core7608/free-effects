#include "../effect_registry.h"
#include "smoke_effect.h"
#include <cmath>
#include <vector>
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<SmokeEffect> s_reg("Smoke", "Simulation");

SmokeEffect::SmokeEffect() {
    addParameter(EffectParameter::makeFloat("density", "Density", 0.0, 1.0, 0.5));
    addParameter(EffectParameter::makeFloat("speed", "Rise Speed", 0.1, 3.0, 0.8));
    addParameter(EffectParameter::makeFloat("turbulence", "Turbulence", 0.0, 5.0, 2.0));
    addParameter(EffectParameter::makeColor("color", "Smoke Color", Color{0.5, 0.5, 0.5, 0.4}));
}

std::vector<ParameterGroup> SmokeEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeFloat("density", "Density", 0.0, 1.0, 0.5),
        EffectParameter::makeFloat("speed", "Rise Speed", 0.1, 3.0, 0.8),
        EffectParameter::makeFloat("turbulence", "Turbulence", 0.0, 5.0, 2.0),
        EffectParameter::makeColor("color", "Smoke Color", Color{0.5, 0.5, 0.5, 0.4})
    }}};
}

std::unique_ptr<Effect> SmokeEffect::clone() const {
    auto e = std::make_unique<SmokeEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void SmokeEffect::render(PixelBuffer& buffer, double time) {
    if (buffer.width == 0 || buffer.height == 0) return;
    double density = getFloatParam("density");
    double spd = getFloatParam("speed");
    double turb = getFloatParam("turbulence");
    Color sc = getColorParam("color");
    double cr = sc.r * 255.0;
    double cg = sc.g * 255.0;
    double cb = sc.b * 255.0;
    for (int y = 0; y < buffer.height; y++) {
        double ny = static_cast<double>(y) / buffer.height;
        double rise = std::fmod(time * spd * 0.3 + ny * 2.0, 3.0);
        double wisps = std::sin(ny * 8.0 + time * 0.5 + turb * std::sin(time + ny * 3.0)) * 0.5 + 0.5;
        wisps *= std::sin(ny * 5.3 - time * 0.3 + turb * std::cos(time * 0.7 + ny * 2.0)) * 0.5 + 0.5;
        double alpha = density * wisps * (1.0 - std::abs(ny - 0.5) * 1.5);
        alpha = std::max(0.0, alpha) * sc.a;
        if (alpha < 0.01) continue;
        for (int x = 0; x < buffer.width; x++) {
            double nx = static_cast<double>(x) / buffer.width;
            double lateral = std::sin(nx * 6.0 + time * 0.8 + turb * std::sin(time * 0.5 + nx * 4.0)) * 0.3 + 0.7;
            double fa = alpha * lateral;
            if (fa < 0.01) continue;
            uint8_t* p = buffer.pixelAt(x, y);
            double sa = p[3] / 255.0;
            double outA = sa + fa * (1.0 - sa);
            if (outA > 0.001) {
                p[0] = static_cast<uint8_t>(std::clamp((p[0] * sa + cr * fa * (1.0 - sa)) / outA, 0.0, 255.0));
                p[1] = static_cast<uint8_t>(std::clamp((p[1] * sa + cg * fa * (1.0 - sa)) / outA, 0.0, 255.0));
                p[2] = static_cast<uint8_t>(std::clamp((p[2] * sa + cb * fa * (1.0 - sa)) / outA, 0.0, 255.0));
            }
            p[3] = static_cast<uint8_t>(std::clamp(outA * 255.0, 0.0, 255.0));
        }
    }
}

} // namespace FreeEffect
