#include "../effect_registry.h"
#include "energy_field_effect.h"
#include <cmath>
#include <vector>
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<EnergyFieldEffect> s_reg("Energy Field", "Simulation");

EnergyFieldEffect::EnergyFieldEffect() {
    addParameter(EffectParameter::makeFloat("intensity", "Intensity", 0.0, 1.0, 0.6));
    addParameter(EffectParameter::makeFloat("frequency", "Frequency", 0.5, 10.0, 3.0));
    addParameter(EffectParameter::makeFloat("speed", "Speed", 0.1, 5.0, 1.5));
    addParameter(EffectParameter::makeColor("color", "Energy Color", Color{0.0, 0.5, 1.0, 0.6}));
    addParameter(EffectParameter::makeFloat("thickness", "Thickness", 0.5, 5.0, 1.5));
    addParameter(EffectParameter::makeInt("arcs", "Arc Count", 1, 20, 5));
}

std::vector<ParameterGroup> EnergyFieldEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeFloat("intensity", "Intensity", 0.0, 1.0, 0.6),
        EffectParameter::makeFloat("frequency", "Frequency", 0.5, 10.0, 3.0),
        EffectParameter::makeFloat("speed", "Speed", 0.1, 5.0, 1.5),
        EffectParameter::makeColor("color", "Energy Color", Color{0.0, 0.5, 1.0, 0.6}),
        EffectParameter::makeFloat("thickness", "Thickness", 0.5, 5.0, 1.5),
        EffectParameter::makeInt("arcs", "Arc Count", 1, 20, 5)
    }}};
}

std::unique_ptr<Effect> EnergyFieldEffect::clone() const {
    auto e = std::make_unique<EnergyFieldEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void EnergyFieldEffect::render(PixelBuffer& buffer, double time) {
    if (buffer.width == 0 || buffer.height == 0) return;
    double intensity = getFloatParam("intensity");
    double freq = getFloatParam("frequency");
    double spd = getFloatParam("speed");
    Color ec = getColorParam("color");
    double thick = getFloatParam("thickness");
    int arcs = getIntParam("arcs");
    double cr = ec.r * 255.0;
    double cg = ec.g * 255.0;
    double cb = ec.b * 255.0;
    double cx = buffer.width / 2.0;
    double cy = buffer.height / 2.0;
    double maxR = std::min(cx, cy) * 0.8;
    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            double dx = (x - cx) / maxR;
            double dy = (y - cy) / maxR;
            double dist = std::sqrt(dx * dx + dy * dy);
            if (dist > 1.2) continue;
            double angle = std::atan2(dy, dx);
            double energy = 0.0;
            for (int a = 0; a < arcs; a++) {
                double arcSeed = std::sin(a * 127.1 + 311.7) * 43758.5453;
                double arcPhase = (arcSeed - std::floor(arcSeed)) * 6.28318;
                double arcAngle = angle + arcPhase + time * spd * 0.5;
                double wave = std::sin(arcAngle * freq + time * spd * 2.0 + dist * 5.0) * 0.5 + 0.5;
                double arcMask = std::exp(-std::abs(dist - 0.3 - wave * 0.4) * 8.0 / thick);
                energy = std::max(energy, arcMask * wave);
            }
            double noise = std::sin(x * 0.1 + time * spd) * std::sin(y * 0.1 - time * spd * 0.7) * 0.2 + 0.8;
            double fa = energy * noise * intensity * ec.a;
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
