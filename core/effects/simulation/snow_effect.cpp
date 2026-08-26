#include "../effect_registry.h"
#include "snow_effect.h"
#include <cmath>
#include <vector>
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<SnowEffect> s_reg("Snow", "Simulation");

SnowEffect::SnowEffect() {
    addParameter(EffectParameter::makeFloat("intensity", "Intensity", 0.0, 1.0, 0.5));
    addParameter(EffectParameter::makeFloat("speed", "Speed", 0.1, 3.0, 0.8));
    addParameter(EffectParameter::makeFloat("wind", "Wind", -2.0, 2.0, 0.3));
    addParameter(EffectParameter::makeInt("flakes", "Flake Count", 10, 2000, 600));
    addParameter(EffectParameter::makeFloat("size", "Flake Size", 1.0, 8.0, 3.0));
    addParameter(EffectParameter::makeColor("color", "Flake Color", Color{1.0, 1.0, 1.0, 0.8}));
}

std::vector<ParameterGroup> SnowEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeFloat("intensity", "Intensity", 0.0, 1.0, 0.5),
        EffectParameter::makeFloat("speed", "Speed", 0.1, 3.0, 0.8),
        EffectParameter::makeFloat("wind", "Wind", -2.0, 2.0, 0.3),
        EffectParameter::makeInt("flakes", "Flake Count", 10, 2000, 600),
        EffectParameter::makeFloat("size", "Flake Size", 1.0, 8.0, 3.0),
        EffectParameter::makeColor("color", "Flake Color", Color{1.0, 1.0, 1.0, 0.8})
    }}};
}

std::unique_ptr<Effect> SnowEffect::clone() const {
    auto e = std::make_unique<SnowEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void SnowEffect::render(PixelBuffer& buffer, double time) {
    if (buffer.width == 0 || buffer.height == 0) return;
    double intensity = getFloatParam("intensity");
    double spd = getFloatParam("speed");
    double wind = getFloatParam("wind");
    int flakes = getIntParam("flakes");
    double sz = getFloatParam("size");
    Color sc = getColorParam("color");
    int effective = static_cast<int>(flakes * intensity);
    double r = sc.r * 255.0;
    double g = sc.g * 255.0;
    double b = sc.b * 255.0;
    double a = sc.a;
    for (int i = 0; i < effective; i++) {
        double seed1 = std::sin(i * 127.1 + 311.7) * 43758.5453;
        double seed2 = std::sin(i * 269.5 + 183.3) * 43758.5453;
        double seed3 = std::sin(i * 419.2 + 371.9) * 43758.5453;
        double f1 = seed1 - std::floor(seed1);
        double f2 = seed2 - std::floor(seed2);
        double f3 = seed3 - std::floor(seed3);
        double baseX = f1 * buffer.width;
        double wobblePhase = f2 * 6.28318;
        double fallSpeed = (0.5 + f3 * 0.5) * spd * 100.0;
        double yPos = std::fmod(time * fallSpeed + f1 * buffer.height * 3.0, buffer.height + 40.0) - 20.0;
        double xWobble = std::sin(time * 1.5 + wobblePhase) * 15.0;
        double xPos = baseX + xWobble + wind * time * 50.0;
        xPos = std::fmod(xPos + buffer.width * 2.0, static_cast<double>(buffer.width + 40.0)) - 20.0;
        int px = static_cast<int>(std::round(xPos));
        int py = static_cast<int>(std::round(yPos));
        int radius = static_cast<int>(std::ceil(sz * (0.5 + f3 * 0.5)));
        for (int dy = -radius; dy <= radius; dy++) {
            for (int dx = -radius; dx <= radius; dx++) {
                int sx = px + dx;
                int sy = py + dy;
                if (sx < 0 || sx >= buffer.width || sy < 0 || sy >= buffer.height) continue;
                double dist = std::sqrt(dx * dx + dy * dy);
                if (dist > radius) continue;
                double falloff = 1.0 - dist / (radius + 0.5);
                uint8_t* p = buffer.pixelAt(sx, sy);
                double sa = p[3] / 255.0;
                double da = a * falloff;
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
}

} // namespace FreeEffect
