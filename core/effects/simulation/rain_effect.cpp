#include "../effect_registry.h"
#include "rain_effect.h"
#include <cmath>
#include <vector>
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<RainEffect> s_reg("Rain", "Simulation");

RainEffect::RainEffect() {
    addParameter(EffectParameter::makeFloat("intensity", "Intensity", 0.0, 1.0, 0.5));
    addParameter(EffectParameter::makeFloat("speed", "Speed", 0.1, 5.0, 1.0));
    addParameter(EffectParameter::makeFloat("angle", "Angle", 0.0, 90.0, 15.0));
    addParameter(EffectParameter::makeInt("drops", "Drop Count", 10, 2000, 500));
    addParameter(EffectParameter::makeFloat("length", "Drop Length", 1.0, 30.0, 10.0));
    addParameter(EffectParameter::makeColor("color", "Drop Color", Color{0.7, 0.7, 0.8, 0.6}));
}

std::vector<ParameterGroup> RainEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeFloat("intensity", "Intensity", 0.0, 1.0, 0.5),
        EffectParameter::makeFloat("speed", "Speed", 0.1, 5.0, 1.0),
        EffectParameter::makeFloat("angle", "Angle", 0.0, 90.0, 15.0),
        EffectParameter::makeInt("drops", "Drop Count", 10, 2000, 500),
        EffectParameter::makeFloat("length", "Drop Length", 1.0, 30.0, 10.0),
        EffectParameter::makeColor("color", "Drop Color", Color{0.7, 0.7, 0.8, 0.6})
    }}};
}

std::unique_ptr<Effect> RainEffect::clone() const {
    auto e = std::make_unique<RainEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void RainEffect::render(PixelBuffer& buffer, double time) {
    if (buffer.width == 0 || buffer.height == 0) return;
    double intensity = getFloatParam("intensity");
    double spd = getFloatParam("speed");
    double ang = getFloatParam("angle") * M_PI / 180.0;
    int drops = getIntParam("drops");
    double len = getFloatParam("length");
    Color rc = getColorParam("color");
    int effectiveDrops = static_cast<int>(drops * intensity);
    double cosA = std::cos(ang);
    double sinA = std::sin(ang);
    double r = rc.r * 255.0;
    double g = rc.g * 255.0;
    double b = rc.b * 255.0;
    double a = rc.a;
    for (int i = 0; i < effectiveDrops; i++) {
        double seed1 = std::sin(i * 127.1 + 311.7) * 43758.5453;
        double seed2 = std::sin(i * 269.5 + 183.3) * 43758.5453;
        double seed3 = std::sin(i * 419.2 + 371.9) * 43758.5453;
        double f1 = seed1 - std::floor(seed1);
        double f2 = seed2 - std::floor(seed2);
        double f3 = seed3 - std::floor(seed3);
        double baseX = f1 * (buffer.width + 200.0) - 100.0;
        double baseY = f2 * (buffer.height + 200.0) - 100.0;
        double phase = f3 * 10.0;
        double yOffset = std::fmod(time * spd * 500.0 + phase * 100.0, buffer.height + 200.0) - 100.0;
        double dropX = baseX + yOffset * sinA;
        double dropY = baseY + yOffset * cosA;
        double dropLen = len * (0.5 + f1 * 0.5);
        int x0 = static_cast<int>(std::round(dropX));
        int y0 = static_cast<int>(std::round(dropY));
        int x1 = static_cast<int>(std::round(dropX - dropLen * sinA));
        int y1 = static_cast<int>(std::round(dropY - dropLen * cosA));
        int steps = std::max(std::abs(x1 - x0), std::abs(y1 - y0)) + 1;
        for (int s = 0; s < steps; s++) {
            double t = steps > 1 ? static_cast<double>(s) / (steps - 1) : 0.0;
            int px = static_cast<int>(std::round(x0 + (x1 - x0) * t));
            int py = static_cast<int>(std::round(y0 + (y1 - y0) * t));
            if (px < 0 || px >= buffer.width || py < 0 || py >= buffer.height) continue;
            double fade = 1.0 - t;
            uint8_t* p = buffer.pixelAt(px, py);
            double sa = p[3] / 255.0;
            double da = a * fade;
            double outA = sa + da * (1.0 - sa);
            if (outA > 0.001) {
                p[0] = static_cast<uint8_t>((p[0] * sa + r * da * (1.0 - sa)) / outA);
                p[1] = static_cast<uint8_t>((p[1] * sa + g * da * (1.0 - sa)) / outA);
                p[2] = static_cast<uint8_t>((p[2] * sa + b * da * (1.0 - sa)) / outA);
            }
            p[3] = static_cast<uint8_t>(std::clamp(outA * 255.0, 0.0, 255.0));
        }
    }
}

} // namespace FreeEffect
