#include "../effect_registry.h"
#include "strobe_effect.h"
#include <cmath>
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<StrobeEffect> s_reg("Strobe", "Stylize");

StrobeEffect::StrobeEffect() {
    addParameter(EffectParameter::makeFloat("frequency", "Frequency", 0.1, 30.0, 5.0));
    addParameter(EffectParameter::makeFloat("duty_cycle", "Duty Cycle", 0.0, 1.0, 0.5));
    addParameter(EffectParameter::makeColor("color", "Strobe Color", Color{1.0, 1.0, 1.0, 1.0}));
    addParameter(EffectParameter::makeFloat("intensity", "Intensity", 0.0, 1.0, 1.0));
}

std::vector<ParameterGroup> StrobeEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeFloat("frequency", "Frequency", 0.1, 30.0, 5.0),
        EffectParameter::makeFloat("duty_cycle", "Duty Cycle", 0.0, 1.0, 0.5),
        EffectParameter::makeColor("color", "Strobe Color", Color{1.0, 1.0, 1.0, 1.0}),
        EffectParameter::makeFloat("intensity", "Intensity", 0.0, 1.0, 1.0)
    }}};
}

std::unique_ptr<Effect> StrobeEffect::clone() const {
    auto e = std::make_unique<StrobeEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void StrobeEffect::render(PixelBuffer& buffer, double time) {
    if (buffer.width == 0 || buffer.height == 0) return;
    double freq = getFloatParam("frequency");
    double duty = getFloatParam("duty_cycle");
    Color sc = getColorParam("color");
    double intensity = getFloatParam("intensity");
    double period = 1.0 / freq;
    double phase = std::fmod(time, period) / period;
    double strobeAlpha = phase < duty ? intensity : 0.0;
    if (strobeAlpha < 0.01) return;
    double cr = sc.r * 255.0;
    double cg = sc.g * 255.0;
    double cb = sc.b * 255.0;
    double ca = sc.a * strobeAlpha;
    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            double sa = p[3] / 255.0;
            double outA = sa + ca * (1.0 - sa);
            if (outA > 0.001) {
                p[0] = static_cast<uint8_t>(std::clamp((p[0] * sa + cr * ca * (1.0 - sa)) / outA, 0.0, 255.0));
                p[1] = static_cast<uint8_t>(std::clamp((p[1] * sa + cg * ca * (1.0 - sa)) / outA, 0.0, 255.0));
                p[2] = static_cast<uint8_t>(std::clamp((p[2] * sa + cb * ca * (1.0 - sa)) / outA, 0.0, 255.0));
            }
            p[3] = static_cast<uint8_t>(std::clamp(outA * 255.0, 0.0, 255.0));
        }
    }
}

} // namespace FreeEffect
