#include "../../math/math_constants.h"
#include "../effect_registry.h"
#include "particle_burst_effect.h"
#include <cmath>
#include <vector>
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<ParticleBurstEffect> s_reg("Particle Burst", "Simulation");

ParticleBurstEffect::ParticleBurstEffect() {
    addParameter(EffectParameter::makeFloat("spread", "Spread", 0.0, 360.0, 180.0));
    addParameter(EffectParameter::makeFloat("speed", "Speed", 0.1, 10.0, 2.0));
    addParameter(EffectParameter::makeInt("count", "Particle Count", 10, 1000, 200));
    addParameter(EffectParameter::makeFloat("size", "Particle Size", 1.0, 20.0, 3.0));
    addParameter(EffectParameter::makeColor("color", "Particle Color", Color{1.0, 0.8, 0.0, 1.0}));
    addParameter(EffectParameter::makeFloat("decay", "Decay", 0.5, 5.0, 2.0));
}

std::vector<ParameterGroup> ParticleBurstEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeFloat("spread", "Spread", 0.0, 360.0, 180.0),
        EffectParameter::makeFloat("speed", "Speed", 0.1, 10.0, 2.0),
        EffectParameter::makeInt("count", "Particle Count", 10, 1000, 200),
        EffectParameter::makeFloat("size", "Particle Size", 1.0, 20.0, 3.0),
        EffectParameter::makeColor("color", "Particle Color", Color{1.0, 0.8, 0.0, 1.0}),
        EffectParameter::makeFloat("decay", "Decay", 0.5, 5.0, 2.0)
    }}};
}

std::unique_ptr<Effect> ParticleBurstEffect::clone() const {
    auto e = std::make_unique<ParticleBurstEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void ParticleBurstEffect::render(PixelBuffer& buffer, double time) {
    if (buffer.width == 0 || buffer.height == 0) return;
    double spread = getFloatParam("spread") * M_PI / 180.0;
    double speed = getFloatParam("speed");
    int count = getIntParam("count");
    double psize = getFloatParam("size");
    Color pcolor = getColorParam("color");
    double decay = getFloatParam("decay");
    double cx = buffer.width / 2.0;
    double cy = buffer.height / 2.0;
    std::vector<uint8_t> overlay(buffer.width * buffer.height * 4, 0);
    for (int i = 0; i < count; i++) {
        double seed1 = std::sin(i * 127.1 + 311.7) * 43758.5453;
        double seed2 = std::sin(i * 269.5 + 183.3) * 43758.5453;
        double frac1 = seed1 - std::floor(seed1);
        double frac2 = seed2 - std::floor(seed2);
        double angle = (frac1 - 0.5) * spread;
        double vel = frac2 * speed * 50.0;
        double px = cx + std::cos(angle) * vel * time;
        double py = cy + std::sin(angle) * vel * time;
        double life = std::exp(-decay * time);
        double alpha = life;
        if (alpha < 0.01) continue;
        int radius = static_cast<int>(std::ceil(psize * life));
        double pr = pcolor.r * 255.0 * alpha;
        double pg = pcolor.g * 255.0 * alpha;
        double pb = pcolor.b * 255.0 * alpha;
        double pa = alpha * 255.0;
        for (int dy = -radius; dy <= radius; dy++) {
            for (int dx = -radius; dx <= radius; dx++) {
                int sx = static_cast<int>(std::round(px + dx));
                int sy = static_cast<int>(std::round(py + dy));
                if (sx < 0 || sx >= buffer.width || sy < 0 || sy >= buffer.height) continue;
                double dist = std::sqrt(dx * dx + dy * dy);
                if (dist > radius) continue;
                double falloff = 1.0 - dist / (radius + 1);
                int idx = (sy * buffer.width + sx) * 4;
                overlay[idx + 0] = static_cast<uint8_t>(std::clamp(overlay[idx + 0] + pr * falloff, 0.0, 255.0));
                overlay[idx + 1] = static_cast<uint8_t>(std::clamp(overlay[idx + 1] + pg * falloff, 0.0, 255.0));
                overlay[idx + 2] = static_cast<uint8_t>(std::clamp(overlay[idx + 2] + pb * falloff, 0.0, 255.0));
                overlay[idx + 3] = static_cast<uint8_t>(std::clamp(overlay[idx + 3] + pa * falloff, 0.0, 255.0));
            }
        }
    }
    for (int i = 0; i < buffer.width * buffer.height; i++) {
        int idx = i * 4;
        double srcA = buffer.data[idx + 3] / 255.0;
        double ovrA = overlay[idx + 3] / 255.0;
        double outA = srcA + ovrA * (1.0 - srcA);
        if (outA > 0.001) {
            buffer.data[idx + 0] = static_cast<uint8_t>((buffer.data[idx + 0] * srcA + overlay[idx + 0] * ovrA * (1.0 - srcA)) / outA);
            buffer.data[idx + 1] = static_cast<uint8_t>((buffer.data[idx + 1] * srcA + overlay[idx + 1] * ovrA * (1.0 - srcA)) / outA);
            buffer.data[idx + 2] = static_cast<uint8_t>((buffer.data[idx + 2] * srcA + overlay[idx + 2] * ovrA * (1.0 - srcA)) / outA);
        }
        buffer.data[idx + 3] = static_cast<uint8_t>(std::clamp(outA * 255.0, 0.0, 255.0));
    }
}

} // namespace FreeEffect
