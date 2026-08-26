#include "../effect_registry.h"
#include "optical_flare_effect.h"
#include <cmath>
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<OpticalFlareEffect> s_reg("Optical Flare", "Plugin Effect", "Lens Effects");

OpticalFlareEffect::OpticalFlareEffect() {
    addParameter(EffectParameter::makeVec2("position", "Position", Vec2{0.5, 0.5}));
    addParameter(EffectParameter::makeFloat("brightness", "Brightness", 0.0, 3.0, 1.0));
    addParameter(EffectParameter::makeFloat("size", "Size", 0.01, 2.0, 0.3));
    addParameter(EffectParameter::makeColor("tint", "Tint", Color{1.0, 0.9, 0.7, 1.0}));
    addParameter(EffectParameter::makeFloat("rays", "Ray Length", 0.0, 1.0, 0.3));
    addParameter(EffectParameter::makeInt("ray_count", "Ray Count", 0, 16, 6));
    addParameter(EffectParameter::makeFloat("ring_size", "Ring Size", 0.0, 1.0, 0.4));
    addParameter(EffectParameter::makeFloat("ring_thickness", "Ring Thickness", 0.0, 0.1, 0.02));
}

std::vector<ParameterGroup> OpticalFlareEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeVec2("position", "Position", Vec2{0.5, 0.5}),
        EffectParameter::makeFloat("brightness", "Brightness", 0.0, 3.0, 1.0),
        EffectParameter::makeFloat("size", "Size", 0.01, 2.0, 0.3),
        EffectParameter::makeColor("tint", "Tint", Color{1.0, 0.9, 0.7, 1.0}),
        EffectParameter::makeFloat("rays", "Ray Length", 0.0, 1.0, 0.3),
        EffectParameter::makeInt("ray_count", "Ray Count", 0, 16, 6),
        EffectParameter::makeFloat("ring_size", "Ring Size", 0.0, 1.0, 0.4),
        EffectParameter::makeFloat("ring_thickness", "Ring Thickness", 0.0, 0.1, 0.02)
    }}};
}

std::unique_ptr<Effect> OpticalFlareEffect::clone() const {
    auto e = std::make_unique<OpticalFlareEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void OpticalFlareEffect::render(PixelBuffer& buffer, double time) {
    if (buffer.width == 0 || buffer.height == 0) return;
    Vec2 pos = getVec2Param("position");
    double brightness = getFloatParam("brightness");
    double sz = getFloatParam("size");
    Color tc = getColorParam("tint");
    double rayLen = getFloatParam("rays");
    int rayCount = getIntParam("ray_count");
    double ringSz = getFloatParam("ring_size");
    double ringThick = getFloatParam("ring_thickness");
    double cx = pos.x * buffer.width;
    double cy = pos.y * buffer.height;
    double maxR = sz * std::max(buffer.width, buffer.height);
    double cr = tc.r * 255.0;
    double cg = tc.g * 255.0;
    double cb = tc.b * 255.0;
    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            double dx = (x - cx) / maxR;
            double dy = (y - cy) / maxR;
            double dist = std::sqrt(dx * dx + dy * dy);
            if (dist > 1.5) continue;
            double angle = std::atan2(dy, dx);
            double coreBright = std::exp(-dist * dist * 8.0);
            double haloBright = std::exp(-dist * dist * 2.0) * 0.3;
            double rayBright = 0.0;
            if (rayCount > 0 && rayLen > 0.001) {
                for (int r = 0; r < rayCount; r++) {
                    double rayAngle = r * 6.28318 / rayCount + time * 0.3;
                    double adiff = std::abs(std::fmod(angle - rayAngle + 3.14159, 6.28318) - 3.14159);
                    double ray = std::exp(-adiff * adiff * 20.0) * std::exp(-dist / rayLen * 2.0);
                    rayBright = std::max(rayBright, ray);
                }
                rayBright *= 0.4;
            }
            double ringBright = 0.0;
            if (ringSz > 0.001 && ringThick > 0.001) {
                double ringDist = std::abs(dist - ringSz);
                ringBright = std::exp(-ringDist * ringDist / (ringThick * ringThick) * 50.0) * 0.3;
            }
            double totalBright = (coreBright + haloBright + rayBright + ringBright) * brightness;
            if (totalBright < 0.01) continue;
            double fa = std::min(1.0, totalBright);
            double fr = cr * totalBright;
            double fg = cg * totalBright;
            double fb = cb * totalBright;
            uint8_t* p = buffer.pixelAt(x, y);
            double sa = p[3] / 255.0;
            double outA = sa + fa * (1.0 - sa);
            if (outA > 0.001) {
                p[0] = static_cast<uint8_t>(std::clamp((p[0] * sa + fr * fa * (1.0 - sa)) / outA, 0.0, 255.0));
                p[1] = static_cast<uint8_t>(std::clamp((p[1] * sa + fg * fa * (1.0 - sa)) / outA, 0.0, 255.0));
                p[2] = static_cast<uint8_t>(std::clamp((p[2] * sa + fb * fa * (1.0 - sa)) / outA, 0.0, 255.0));
            }
            p[3] = static_cast<uint8_t>(std::clamp(outA * 255.0, 0.0, 255.0));
        }
    }
}

} // namespace FreeEffect
