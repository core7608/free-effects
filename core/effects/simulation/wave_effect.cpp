#include "../effect_registry.h"
#include "wave_effect.h"
#include <cmath>
#include <vector>
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<WaveEffect> s_reg("Wave", "Simulation");

WaveEffect::WaveEffect() {
    addParameter(EffectParameter::makeFloat("amplitude", "Amplitude", 0.0, 50.0, 10.0));
    addParameter(EffectParameter::makeFloat("frequency", "Frequency", 0.01, 0.5, 0.1));
    addParameter(EffectParameter::makeFloat("speed", "Speed", 0.1, 5.0, 1.0));
    addParameter(EffectParameter::makeFloat("depth", "Depth", 0.0, 1.0, 0.6));
    addParameter(EffectParameter::makeColor("color", "Water Color", Color{0.0, 0.3, 0.6, 0.7}));
    addParameter(EffectParameter::makeFloat("foam", "Foam", 0.0, 1.0, 0.3));
}

std::vector<ParameterGroup> WaveEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeFloat("amplitude", "Amplitude", 0.0, 50.0, 10.0),
        EffectParameter::makeFloat("frequency", "Frequency", 0.01, 0.5, 0.1),
        EffectParameter::makeFloat("speed", "Speed", 0.1, 5.0, 1.0),
        EffectParameter::makeFloat("depth", "Depth", 0.0, 1.0, 0.6),
        EffectParameter::makeColor("color", "Water Color", Color{0.0, 0.3, 0.6, 0.7}),
        EffectParameter::makeFloat("foam", "Foam", 0.0, 1.0, 0.3)
    }}};
}

std::unique_ptr<Effect> WaveEffect::clone() const {
    auto e = std::make_unique<WaveEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void WaveEffect::render(PixelBuffer& buffer, double time) {
    if (buffer.width == 0 || buffer.height == 0) return;
    double amp = getFloatParam("amplitude");
    double freq = getFloatParam("frequency");
    double spd = getFloatParam("speed");
    double depth = getFloatParam("depth");
    Color wc = getColorParam("color");
    double foamAmt = getFloatParam("foam");
    for (int y = 0; y < buffer.height; y++) {
        double ny = static_cast<double>(y) / buffer.height;
        double waterMask = std::max(0.0, 1.0 - std::abs(ny - depth) * 4.0);
        if (waterMask < 0.01) continue;
        for (int x = 0; x < buffer.width; x++) {
            double nx = static_cast<double>(x) / buffer.width;
            double wave1 = amp * std::sin(nx * 3.0 + time * spd) * 0.6;
            double wave2 = amp * 0.4 * std::sin(nx * 7.0 - time * spd * 1.3 + 1.0);
            double waveTotal = wave1 + wave2;
            double surfaceDist = std::abs(ny - depth - waveTotal / buffer.height * 0.05);
            double surfaceBright = std::exp(-surfaceDist * 30.0);
            double foam = surfaceBright * foamAmt;
            double fa = waterMask * wc.a * (0.3 + 0.7 * surfaceBright);
            double cr = wc.r * 255.0 * (1.0 - foam) + 255.0 * foam;
            double cg = wc.g * 255.0 * (1.0 - foam) + 255.0 * foam;
            double cb = wc.b * 255.0 * (1.0 - foam) + 255.0 * foam;
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
