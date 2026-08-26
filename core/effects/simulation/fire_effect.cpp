#include "../effect_registry.h"
#include "fire_effect.h"
#include <cmath>
#include <vector>
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<FireEffect> s_reg("Fire", "Simulation");

FireEffect::FireEffect() {
    addParameter(EffectParameter::makeFloat("intensity", "Intensity", 0.0, 1.0, 0.7));
    addParameter(EffectParameter::makeFloat("height", "Flame Height", 0.1, 2.0, 1.0));
    addParameter(EffectParameter::makeFloat("turbulence", "Turbulence", 0.0, 5.0, 2.0));
    addParameter(EffectParameter::makeColor("inner_color", "Inner Color", Color{1.0, 1.0, 0.5, 1.0}));
    addParameter(EffectParameter::makeColor("outer_color", "Outer Color", Color{1.0, 0.2, 0.0, 0.8}));
}

std::vector<ParameterGroup> FireEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeFloat("intensity", "Intensity", 0.0, 1.0, 0.7),
        EffectParameter::makeFloat("height", "Flame Height", 0.1, 2.0, 1.0),
        EffectParameter::makeFloat("turbulence", "Turbulence", 0.0, 5.0, 2.0),
        EffectParameter::makeColor("inner_color", "Inner Color", Color{1.0, 1.0, 0.5, 1.0}),
        EffectParameter::makeColor("outer_color", "Outer Color", Color{1.0, 0.2, 0.0, 0.8})
    }}};
}

std::unique_ptr<Effect> FireEffect::clone() const {
    auto e = std::make_unique<FireEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void FireEffect::render(PixelBuffer& buffer, double time) {
    if (buffer.width == 0 || buffer.height == 0) return;
    double intensity = getFloatParam("intensity");
    double h = getFloatParam("height");
    double turb = getFloatParam("turbulence");
    Color ic = getColorParam("inner_color");
    Color oc = getColorParam("outer_color");
    for (int y = 0; y < buffer.height; y++) {
        double ny = 1.0 - static_cast<double>(y) / buffer.height;
        double flameMask = std::pow(ny, 1.0 / (h * 0.5 + 0.5));
        if (flameMask < 0.01) continue;
        for (int x = 0; x < buffer.width; x++) {
            double nx = static_cast<double>(x) / buffer.width;
            double noise = std::sin(nx * 12.0 + time * 3.0 + ny * turb * 5.0) * 0.5 + 0.5;
            noise *= std::sin(nx * 7.3 - time * 2.1 + ny * turb * 3.7) * 0.5 + 0.5;
            double flicker = std::sin(time * 8.0 + nx * 3.0) * 0.15 + 0.85;
            double flame = flameMask * noise * flicker * intensity;
            if (flame < 0.01) continue;
            double temp = ny * noise;
            double fr = ic.r * (1.0 - temp) + oc.r * temp;
            double fg = ic.g * (1.0 - temp) + oc.g * temp;
            double fb = ic.b * (1.0 - temp) + oc.b * temp;
            double fa = flame * oc.a;
            uint8_t* p = buffer.pixelAt(x, y);
            double sa = p[3] / 255.0;
            double outA = sa + fa * (1.0 - sa);
            if (outA > 0.001) {
                p[0] = static_cast<uint8_t>(std::clamp((p[0] * sa + fr * 255.0 * fa * (1.0 - sa)) / outA, 0.0, 255.0));
                p[1] = static_cast<uint8_t>(std::clamp((p[1] * sa + fg * 255.0 * fa * (1.0 - sa)) / outA, 0.0, 255.0));
                p[2] = static_cast<uint8_t>(std::clamp((p[2] * sa + fb * 255.0 * fa * (1.0 - sa)) / outA, 0.0, 255.0));
            }
            p[3] = static_cast<uint8_t>(std::clamp(outA * 255.0, 0.0, 255.0));
        }
    }
}

} // namespace FreeEffect
