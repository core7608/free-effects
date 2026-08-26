#include "../effect_registry.h"
#include "particular_effect.h"
#include <cmath>
#include <vector>
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<ParticularEffect> s_reg("Particular", "Plugin Effect", "Trapcode");

ParticularEffect::ParticularEffect() {
    addParameter(EffectParameter::makeVec2("emitter_pos", "Emitter Position", Vec2{0.5, 0.5}));
    addParameter(EffectParameter::makeInt("particle_count", "Particles/sec", 10, 5000, 200));
    addParameter(EffectParameter::makeFloat("life", "Particle Life", 0.1, 10.0, 2.0));
    addParameter(EffectParameter::makeFloat("velocity", "Velocity", 0.0, 500.0, 100.0));
    addParameter(EffectParameter::makeFloat("gravity", "Gravity", -500.0, 500.0, 100.0));
    addParameter(EffectParameter::makeFloat("size", "Particle Size", 1.0, 20.0, 3.0));
    addParameter(EffectParameter::makeColor("color", "Color", Color{0.5, 0.7, 1.0, 1.0}));
    addParameter(EffectParameter::makeFloat("fade_in", "Fade In", 0.0, 1.0, 0.1));
    addParameter(EffectParameter::makeFloat("fade_out", "Fade Out", 0.0, 1.0, 0.8));
}

std::vector<ParameterGroup> ParticularEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeVec2("emitter_pos", "Emitter Position", Vec2{0.5, 0.5}),
        EffectParameter::makeInt("particle_count", "Particles/sec", 10, 5000, 200),
        EffectParameter::makeFloat("life", "Particle Life", 0.1, 10.0, 2.0),
        EffectParameter::makeFloat("velocity", "Velocity", 0.0, 500.0, 100.0),
        EffectParameter::makeFloat("gravity", "Gravity", -500.0, 500.0, 100.0),
        EffectParameter::makeFloat("size", "Particle Size", 1.0, 20.0, 3.0),
        EffectParameter::makeColor("color", "Color", Color{0.5, 0.7, 1.0, 1.0}),
        EffectParameter::makeFloat("fade_in", "Fade In", 0.0, 1.0, 0.1),
        EffectParameter::makeFloat("fade_out", "Fade Out", 0.0, 1.0, 0.8)
    }}};
}

std::unique_ptr<Effect> ParticularEffect::clone() const {
    auto e = std::make_unique<ParticularEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void ParticularEffect::render(PixelBuffer& buffer, double time) {
    if (buffer.width == 0 || buffer.height == 0) return;
    Vec2 emitPos = getVec2Param("emitter_pos");
    int pCount = getIntParam("particle_count");
    double life = getFloatParam("life");
    double vel = getFloatParam("velocity");
    double grav = getFloatParam("gravity");
    double sz = getFloatParam("size");
    Color pc = getColorParam("color");
    double fadeIn = getFloatParam("fade_in");
    double fadeOut = getFloatParam("fade_out");
    double ex = emitPos.x * buffer.width;
    double ey = emitPos.y * buffer.height;
    int totalParticles = static_cast<int>(pCount * life) + 1;
    totalParticles = std::min(totalParticles, 5000);
    for (int i = 0; i < totalParticles; i++) {
        double seed1 = std::sin(i * 127.1 + 311.7) * 43758.5453;
        double seed2 = std::sin(i * 269.5 + 183.3) * 43758.5453;
        double seed3 = std::sin(i * 419.2 + 371.9) * 43758.5453;
        double seed4 = std::sin(i * 573.1 + 193.3) * 43758.5453;
        double f1 = seed1 - std::floor(seed1);
        double f2 = seed2 - std::floor(seed2);
        double f3 = seed3 - std::floor(seed3);
        double f4 = seed4 - std::floor(seed4);
        double emitTime = f1 * life * pCount / (pCount * life + 1.0) * life;
        double age = time - emitTime;
        if (age < 0 || age > life) continue;
        double angle = f2 * 6.28318;
        double speed = f3 * vel;
        double px = ex + std::cos(angle) * speed * age;
        double py = ey + std::sin(angle) * speed * age + 0.5 * grav * age * age;
        double ageRatio = age / life;
        double alpha = 1.0;
        if (ageRatio < fadeIn) alpha = ageRatio / fadeIn;
        if (ageRatio > fadeOut) alpha = (1.0 - ageRatio) / (1.0 - fadeOut);
        alpha = std::clamp(alpha, 0.0, 1.0);
        double pSize = sz * (1.0 - ageRatio * 0.5);
        int radius = static_cast<int>(std::ceil(pSize));
        int ipx = static_cast<int>(std::round(px));
        int ipy = static_cast<int>(std::round(py));
        double fr = pc.r * 255.0 * (1.0 + (f4 - 0.5) * 0.2);
        double fg = pc.g * 255.0 * (1.0 + (f4 - 0.5) * 0.2);
        double fb = pc.b * 255.0 * (1.0 + (f4 - 0.5) * 0.2);
        for (int dy = -radius; dy <= radius; dy++) {
            for (int dx = -radius; dx <= radius; dx++) {
                int sx = ipx + dx;
                int sy = ipy + dy;
                if (sx < 0 || sx >= buffer.width || sy < 0 || sy >= buffer.height) continue;
                double dist = std::sqrt(dx * dx + dy * dy);
                if (dist > radius) continue;
                double falloff = 1.0 - dist / (radius + 0.5);
                double fa = alpha * falloff;
                uint8_t* p = buffer.pixelAt(sx, sy);
                double sa = p[3] / 255.0;
                double outA = sa + fa * (1.0 - sa);
                if (outA > 0.001) {
                    p[0] = static_cast<uint8_t>(std::clamp(static_cast<double>((p[0] * sa + fr * fa * (1.0 - sa)) / outA), 0.0, 255.0));
                    p[1] = static_cast<uint8_t>(std::clamp(static_cast<double>((p[1] * sa + fg * fa * (1.0 - sa)) / outA), 0.0, 255.0));
                    p[2] = static_cast<uint8_t>(std::clamp(static_cast<double>((p[2] * sa + fb * fa * (1.0 - sa)) / outA), 0.0, 255.0));
                }
                p[3] = static_cast<uint8_t>(std::clamp(static_cast<double>(outA * 255.0), 0.0, 255.0));
            }
        }
    }
}

} // namespace FreeEffect
