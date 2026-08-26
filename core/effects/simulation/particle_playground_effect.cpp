#include "../effect_registry.h"
#include "particle_playground_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<ParticlePlaygroundEffect> s_reg("Particle Playground", "Simulation");

ParticlePlaygroundEffect::ParticlePlaygroundEffect() {
    addParameter(EffectParameter::makeVec2("cannonPosition", "Cannon Position", {0.5, 0.5}));
    addParameter(EffectParameter::makeFloat("cannonRadius", "Cannon Radius", 0.0, 500.0, 50.0));
    addParameter(EffectParameter::makeFloat("velocity", "Velocity", 0.0, 1000.0, 200.0));
    addParameter(EffectParameter::makeAngle("direction", "Direction", 90.0));
    addParameter(EffectParameter::makeFloat("gravity", "Gravity", -500.0, 500.0, 100.0));
    addParameter(EffectParameter::makeInt("particlesPerSecond", "Particles Per Second", 1, 500, 50));
}

std::unique_ptr<Effect> ParticlePlaygroundEffect::clone() const {
    auto e = std::make_unique<ParticlePlaygroundEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void ParticlePlaygroundEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    Vec2 pos = getVec2Param("cannonPosition");
    float vel = getFloatParam("velocity");
    float dir = getFloatParam("direction") * 3.14159265f / 180.0f;
    float grav = getFloatParam("gravity");
    int pps = getIntParam("particlesPerSecond");

    float cx = pos.x * buffer.width, cy = pos.y * buffer.height;

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            float hash = std::fmod(std::sin(static_cast<float>(x) * 12.9898f + y * 78.233f + static_cast<float>(time)) * 43758.5453f, 1.0f);
            if (hash < 0.01f) {
                float dx = x - cx, dy = y - cy;
                float dist = std::sqrt(dx * dx + dy * dy);
                if (dist < 200.0f) {
                    uint8_t* p = buffer.pixelAt(x, y);
                    float brightness = 1.0f - dist / 200.0f;
                    p[0] = static_cast<uint8_t>(std::min(255.0f, 255.0f * brightness));
                    p[1] = static_cast<uint8_t>(std::min(255.0f, 200.0f * brightness));
                    p[2] = static_cast<uint8_t>(std::min(255.0f, 100.0f * brightness));
                    p[3] = 255;
                }
            }
        }
    }
}

} // namespace FreeEffect
