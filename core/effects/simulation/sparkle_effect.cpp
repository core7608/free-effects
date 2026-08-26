#include "../effect_registry.h"
#include "sparkle_effect.h"
#include <cmath>
#include <vector>
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<SparkleEffect> s_reg("Sparkle", "Simulation");

SparkleEffect::SparkleEffect() {
    addParameter(EffectParameter::makeFloat("intensity", "Intensity", 0.0, 1.0, 0.6));
    addParameter(EffectParameter::makeInt("count", "Sparkle Count", 5, 500, 100));
    addParameter(EffectParameter::makeFloat("speed", "Twinkle Speed", 0.5, 10.0, 3.0));
    addParameter(EffectParameter::makeFloat("size", "Size", 1.0, 15.0, 4.0));
    addParameter(EffectParameter::makeColor("color", "Color", Color{1.0, 1.0, 0.9, 1.0}));
    addParameter(EffectParameter::makeFloat("rays", "Rays", 3.0, 8.0, 4.0));
}

std::vector<ParameterGroup> SparkleEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeFloat("intensity", "Intensity", 0.0, 1.0, 0.6),
        EffectParameter::makeInt("count", "Sparkle Count", 5, 500, 100),
        EffectParameter::makeFloat("speed", "Twinkle Speed", 0.5, 10.0, 3.0),
        EffectParameter::makeFloat("size", "Size", 1.0, 15.0, 4.0),
        EffectParameter::makeColor("color", "Color", Color{1.0, 1.0, 0.9, 1.0}),
        EffectParameter::makeFloat("rays", "Rays", 3.0, 8.0, 4.0)
    }}};
}

std::unique_ptr<Effect> SparkleEffect::clone() const {
    auto e = std::make_unique<SparkleEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void SparkleEffect::render(PixelBuffer& buffer, double time) {
    if (buffer.width == 0 || buffer.height == 0) return;
    double intensity = getFloatParam("intensity");
    int count = getIntParam("count");
    double spd = getFloatParam("speed");
    double sz = getFloatParam("size");
    Color sc = getColorParam("color");
    double numRays = getFloatParam("rays");
    double cr = sc.r * 255.0;
    double cg = sc.g * 255.0;
    double cb = sc.b * 255.0;
    int effective = static_cast<int>(count * intensity);
    for (int i = 0; i < effective; i++) {
        double seed1 = std::sin(i * 127.1 + 311.7) * 43758.5453;
        double seed2 = std::sin(i * 269.5 + 183.3) * 43758.5453;
        double seed3 = std::sin(i * 419.2 + 371.9) * 43758.5453;
        double f1 = seed1 - std::floor(seed1);
        double f2 = seed2 - std::floor(seed2);
        double f3 = seed3 - std::floor(seed3);
        double px = f1 * buffer.width;
        double py = f2 * buffer.height;
        double phase = f3 * 6.28318;
        double twinkle = std::pow(std::max(0.0, std::sin(time * spd * (0.5 + f1 * 2.0) + phase)), 4.0);
        if (twinkle < 0.01) continue;
        double radius = sz * (0.5 + f2 * 0.5) * twinkle;
        int ir = static_cast<int>(std::ceil(radius));
        int ipx = static_cast<int>(std::round(px));
        int ipy = static_cast<int>(std::round(py));
        for (int dy = -ir; dy <= ir; dy++) {
            for (int dx = -ir; dx <= ir; dx++) {
                int sx = ipx + dx;
                int sy = ipy + dy;
                if (sx < 0 || sx >= buffer.width || sy < 0 || sy >= buffer.height) continue;
                double dist = std::sqrt(dx * dx + dy * dy);
                if (dist > radius) continue;
                double angle = std::atan2(dy, dx);
                double rayPattern = 0.0;
                for (int r = 0; r < static_cast<int>(numRays); r++) {
                    double rayAngle = r * 6.28318 / numRays;
                    double adiff = std::abs(std::fmod(angle - rayAngle + 3.14159, 6.28318) - 3.14159);
                    rayPattern = std::max(rayPattern, std::exp(-adiff * adiff * 3.0));
                }
                double falloff = (1.0 - dist / (radius + 0.5)) * (0.3 + 0.7 * rayPattern);
                double fa = twinkle * falloff;
                uint8_t* p = buffer.pixelAt(sx, sy);
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
}

} // namespace FreeEffect
