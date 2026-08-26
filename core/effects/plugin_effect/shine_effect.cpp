#include "../effect_registry.h"
#include "shine_effect.h"
#include <cmath>
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<ShineEffect> s_reg("Shine", "Plugin Effect", "Trapcode");

ShineEffect::ShineEffect() {
    addParameter(EffectParameter::makeVec2("source_point", "Source Point", Vec2{0.5, 0.5}));
    addParameter(EffectParameter::makeFloat("ray_length", "Ray Length", 0.1, 5.0, 1.0));
    addParameter(EffectParameter::makeFloat("intensity", "Intensity", 0.0, 3.0, 1.0));
    addParameter(EffectParameter::makeColor("color", "Color", Color{1.0, 0.8, 0.0, 1.0}));
    addParameter(EffectParameter::makeAngle("direction", "Direction", 0.0));
    addParameter(EffectParameter::makeFloat("spread", "Spread", 0.0, 180.0, 30.0));
}

std::vector<ParameterGroup> ShineEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeVec2("source_point", "Source Point", Vec2{0.5, 0.5}),
        EffectParameter::makeFloat("ray_length", "Ray Length", 0.1, 5.0, 1.0),
        EffectParameter::makeFloat("intensity", "Intensity", 0.0, 3.0, 1.0),
        EffectParameter::makeColor("color", "Color", Color{1.0, 0.8, 0.0, 1.0}),
        EffectParameter::makeAngle("direction", "Direction", 0.0),
        EffectParameter::makeFloat("spread", "Spread", 0.0, 180.0, 30.0)
    }}};
}

std::unique_ptr<Effect> ShineEffect::clone() const {
    auto e = std::make_unique<ShineEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void ShineEffect::render(PixelBuffer& buffer, double time) {
    if (buffer.width == 0 || buffer.height == 0) return;
    Vec2 src = getVec2Param("source_point");
    double rayLen = getFloatParam("ray_length");
    double intensity = getFloatParam("intensity");
    Color sc = getColorParam("color");
    double dir = getAngleParam("direction") * M_PI / 180.0;
    double spread = getFloatParam("spread") * M_PI / 180.0;
    double sx = src.x * buffer.width;
    double sy = src.y * buffer.height;
    double maxDist = std::sqrt(buffer.width * buffer.width + buffer.height * buffer.height);
    double rayPixels = rayLen * maxDist;
    double cr = sc.r * 255.0;
    double cg = sc.g * 255.0;
    double cb = sc.b * 255.0;
    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);
    tmp.data = buffer.data;
    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            double dx = x - sx;
            double dy = y - sy;
            double dist = std::sqrt(dx * dx + dy * dy);
            if (dist < 1 || dist > rayPixels) continue;
            double angle = std::atan2(dy, dx);
            double angleDiff = std::abs(std::fmod(angle - dir + 3.14159, 6.28318) - 3.14159);
            if (angleDiff > spread) continue;
            double angleFade = 1.0 - angleDiff / spread;
            double distFade = 1.0 - dist / rayPixels;
            double bright = angleFade * distFade * intensity;
            const uint8_t* srcP = tmp.pixelAt(
                std::clamp(static_cast<int>(x - dx * 0.05 * bright), 0, buffer.width - 1),
                std::clamp(static_cast<int>(y - dy * 0.05 * bright), 0, buffer.height - 1));
            double srcLuma = (srcP[0] * 0.299 + srcP[1] * 0.587 + srcP[2] * 0.114) / 255.0;
            bright *= (0.3 + srcLuma * 0.7);
            double fa = std::min(1.0, bright * sc.a);
            double fr = cr * bright;
            double fg = cg * bright;
            double fb = cb * bright;
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
