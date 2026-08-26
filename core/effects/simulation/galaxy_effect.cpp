#include "../effect_registry.h"
#include "galaxy_effect.h"
#include <cmath>
#include <vector>
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<GalaxyEffect> s_reg("Galaxy", "Simulation");

GalaxyEffect::GalaxyEffect() {
    addParameter(EffectParameter::makeFloat("intensity", "Intensity", 0.0, 1.0, 0.6));
    addParameter(EffectParameter::makeInt("stars", "Star Count", 50, 5000, 1000));
    addParameter(EffectParameter::makeFloat("rotation", "Rotation Speed", 0.0, 2.0, 0.3));
    addParameter(EffectParameter::makeFloat("arms", "Spiral Arms", 1.0, 8.0, 2.0));
    addParameter(EffectParameter::makeFloat("spread", "Spread", 0.1, 2.0, 0.8));
    addParameter(EffectParameter::makeColor("color", "Tint Color", Color{0.6, 0.5, 1.0, 0.5}));
}

std::vector<ParameterGroup> GalaxyEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeFloat("intensity", "Intensity", 0.0, 1.0, 0.6),
        EffectParameter::makeInt("stars", "Star Count", 50, 5000, 1000),
        EffectParameter::makeFloat("rotation", "Rotation Speed", 0.0, 2.0, 0.3),
        EffectParameter::makeFloat("arms", "Spiral Arms", 1.0, 8.0, 2.0),
        EffectParameter::makeFloat("spread", "Spread", 0.1, 2.0, 0.8),
        EffectParameter::makeColor("color", "Tint Color", Color{0.6, 0.5, 1.0, 0.5})
    }}};
}

std::unique_ptr<Effect> GalaxyEffect::clone() const {
    auto e = std::make_unique<GalaxyEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void GalaxyEffect::render(PixelBuffer& buffer, double time) {
    if (buffer.width == 0 || buffer.height == 0) return;
    double intensity = getFloatParam("intensity");
    int stars = getIntParam("stars");
    double rotSpd = getFloatParam("rotation");
    double arms = getFloatParam("arms");
    double spread = getFloatParam("spread");
    Color tc = getColorParam("color");
    double cx = buffer.width / 2.0;
    double cy = buffer.height / 2.0;
    double maxR = std::min(cx, cy);
    double cr = tc.r * 255.0;
    double cg = tc.g * 255.0;
    double cb = tc.b * 255.0;
    for (int i = 0; i < stars; i++) {
        double seed1 = std::sin(i * 127.1 + 311.7) * 43758.5453;
        double seed2 = std::sin(i * 269.5 + 183.3) * 43758.5453;
        double seed3 = std::sin(i * 419.2 + 371.9) * 43758.5453;
        double seed4 = std::sin(i * 573.1 + 193.3) * 43758.5453;
        double f1 = seed1 - std::floor(seed1);
        double f2 = seed2 - std::floor(seed2);
        double f3 = seed3 - std::floor(seed3);
        double f4 = seed4 - std::floor(seed4);
        double dist = f1 * spread * maxR;
        double armAngle = f2 * arms;
        double spiralAngle = armAngle + dist / maxR * 3.0 + time * rotSpd * (1.0 + f3 * 0.5);
        double spiralSpread = (f3 - 0.5) * dist * 0.15;
        double px = cx + std::cos(spiralAngle) * dist + std::cos(spiralAngle + 1.5708) * spiralSpread;
        double py = cy + std::sin(spiralAngle) * dist + std::sin(spiralAngle + 1.5708) * spiralSpread;
        int ipx = static_cast<int>(std::round(px));
        int ipy = static_cast<int>(std::round(py));
        if (ipx < 0 || ipx >= buffer.width || ipy < 0 || ipy >= buffer.height) continue;
        double brightness = (0.3 + f4 * 0.7) * intensity;
        double coreGlow = std::exp(-dist / (maxR * 0.2)) * 0.3;
        double sr = std::clamp(cr * 0.5 + 255.0 * coreGlow + (180.0 + f4 * 75.0) * brightness * 0.3, 0.0, 255.0);
        double sg = std::clamp(cg * 0.5 + 200.0 * coreGlow + (180.0 + f4 * 75.0) * brightness * 0.3, 0.0, 255.0);
        double sb = std::clamp(cb * 0.5 + 255.0 * coreGlow + (200.0 + f4 * 55.0) * brightness * 0.5, 0.0, 255.0);
        double sa = std::clamp(brightness * 0.8 + coreGlow, 0.0, 1.0);
        int size = static_cast<int>(1 + f4 * 2);
        for (int dy = -size; dy <= size; dy++) {
            for (int dx = -size; dx <= size; dx++) {
                int sx = ipx + dx;
                int sy = ipy + dy;
                if (sx < 0 || sx >= buffer.width || sy < 0 || sy >= buffer.height) continue;
                double d = std::sqrt(dx * dx + dy * dy);
                if (d > size) continue;
                double falloff = 1.0 - d / (size + 0.5);
                double fa = sa * falloff;
                uint8_t* p = buffer.pixelAt(sx, sy);
                double origA = p[3] / 255.0;
                double outA = origA + fa * (1.0 - origA);
                if (outA > 0.001) {
                    p[0] = static_cast<uint8_t>(std::clamp((p[0] * origA + sr * fa * (1.0 - origA)) / outA, 0.0, 255.0));
                    p[1] = static_cast<uint8_t>(std::clamp((p[1] * origA + sg * fa * (1.0 - origA)) / outA, 0.0, 255.0));
                    p[2] = static_cast<uint8_t>(std::clamp((p[2] * origA + sb * fa * (1.0 - origA)) / outA, 0.0, 255.0));
                }
                p[3] = static_cast<uint8_t>(std::clamp(outA * 255.0, 0.0, 255.0));
            }
        }
    }
}

} // namespace FreeEffect
