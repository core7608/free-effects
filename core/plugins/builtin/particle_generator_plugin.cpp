#include "particle_generator_plugin.h"
#include <algorithm>
#include <cmath>
#include <cstring>

namespace FreeEffect {

#ifndef FREEEFFECT_BUILTIN_PLUGIN
extern "C" PluginInterface* createPlugin() {
    return new ParticleGeneratorPlugin();
}

extern "C" void destroyPlugin(PluginInterface* plugin) {
    delete plugin;
}
#endif

ParticleGeneratorPlugin::ParticleGeneratorPlugin()
    : m_rng(std::random_device{}()) {
    m_parameters.push_back({"count", "Particle Count", "Number of particles", 1, 10000, 500, 500, true, PluginParameter::Type::Int});
    m_parameters.push_back({"speed", "Speed", "Particle speed", 0.0, 500.0, 50.0, 50.0, true, PluginParameter::Type::Float});
    m_parameters.push_back({"gravity", "Gravity", "Gravitational pull", -200.0, 200.0, 50.0, 50.0, true, PluginParameter::Type::Float});
    m_parameters.push_back({"size", "Size", "Particle size", 0.5, 50.0, 3.0, 3.0, true, PluginParameter::Type::Float});
    m_parameters.push_back({"spread", "Spread", "Emission spread angle", 0.0, 360.0, 30.0, 30.0, true, PluginParameter::Type::Float});
    m_parameters.push_back({"lifetime", "Lifetime", "Particle lifetime in seconds", 0.1, 10.0, 2.0, 2.0, true, PluginParameter::Type::Float});
    m_parameters.push_back({"colorR", "Color Red", "Particle red", 0.0, 1.0, 1.0, 1.0, true, PluginParameter::Type::Float});
    m_parameters.push_back({"colorG", "Color Green", "Particle green", 0.0, 1.0, 0.5, 0.5, true, PluginParameter::Type::Float});
    m_parameters.push_back({"colorB", "Color Blue", "Particle blue", 0.0, 1.0, 0.0, 0.0, true, PluginParameter::Type::Float});
    m_parameters.push_back({"fadeOut", "Fade Out", "Fade particles as they age", 0.0, 1.0, 1.0, 1.0, true, PluginParameter::Type::Float});
    m_parameters.push_back({"preset", "Preset", "Particle preset", 0, 5, 0, 0, true, PluginParameter::Type::Enum});
    m_parameters.back().enumOptions = {"Fire", "Smoke", "Snow", "Rain", "Sparkles", "Custom"};

    m_parameters.push_back({"emitterX", "Emitter X", "Emitter X position", 0.0, 1.0, 0.5, 0.5, true, PluginParameter::Type::Float});
    m_parameters.push_back({"emitterY", "Emitter Y", "Emitter Y position", 0.0, 1.0, 0.5, 0.5, true, PluginParameter::Type::Float});
    m_parameters.push_back({"turbulence", "Turbulence", "Random motion amount", 0.0, 100.0, 5.0, 5.0, true, PluginParameter::Type::Float});
    m_parameters.push_back({"rotation", "Rotation", "Particle rotation speed", 0.0, 360.0, 0.0, 0.0, true, PluginParameter::Type::Float});
    m_parameters.push_back({"sizeDecay", "Size Decay", "Size decrease over lifetime", 0.0, 1.0, 0.0, 0.0, true, PluginParameter::Type::Float});
    m_parameters.push_back({"windX", "Wind X", "Horizontal wind", -100.0, 100.0, 0.0, 0.0, true, PluginParameter::Type::Float});
    m_parameters.push_back({"windY", "Wind Y", "Vertical wind", -100.0, 100.0, 0.0, 0.0, true, PluginParameter::Type::Float});
}

ParticleGeneratorPlugin::~ParticleGeneratorPlugin() {
    shutdown();
}

PluginResult ParticleGeneratorPlugin::initialize() {
    m_initialized = true;
    m_lastTime = -1.0;
    return PluginResult::ok();
}

PluginResult ParticleGeneratorPlugin::shutdown() {
    m_particles.clear();
    m_initialized = false;
    return PluginResult::ok();
}

const char* ParticleGeneratorPlugin::getName() const { return "Particle Generator"; }
const char* ParticleGeneratorPlugin::getVersion() const { return "1.0.0"; }
const char* ParticleGeneratorPlugin::getDescription() const { return "GPU-accelerated particle system generator with presets"; }
const char* ParticleGeneratorPlugin::getAuthor() const { return "FreeEffect Team"; }
PluginAPIVersion ParticleGeneratorPlugin::getAPIVersion() const { return PluginAPIVersion::v1_0; }
PluginType ParticleGeneratorPlugin::getType() const { return PluginType::Generator; }

uint32_t ParticleGeneratorPlugin::getParameterCount() const {
    return static_cast<uint32_t>(m_parameters.size());
}

PluginParameter ParticleGeneratorPlugin::getParameter(uint32_t index) const {
    if (index < m_parameters.size()) {
        return m_parameters[index];
    }
    return {};
}

PluginResult ParticleGeneratorPlugin::setParameter(const std::string& name, double value) {
    for (auto& param : m_parameters) {
        if (param.name == name) {
            param.currentValue = std::clamp(value, param.minValue, param.maxValue);
            if (name == "preset") {
                applyPreset(static_cast<ParticlePreset>(static_cast<int>(param.currentValue)));
            }
            return PluginResult::ok();
        }
    }
    return PluginResult::error("Unknown parameter: " + name);
}

double ParticleGeneratorPlugin::getParameter(const std::string& name) const {
    for (const auto& param : m_parameters) {
        if (param.name == name) {
            return param.currentValue;
        }
    }
    return 0.0;
}

bool ParticleGeneratorPlugin::needsProcessing() const { return true; }

PluginResult ParticleGeneratorPlugin::process(float* buffer, int width, int height, int channels, double time) {
    if (!buffer || width <= 0 || height <= 0) {
        return PluginResult::error("Invalid buffer");
    }

    float dt = 1.0f / 30.0f;
    if (m_lastTime >= 0.0) {
        dt = static_cast<float>(time - m_lastTime);
    }
    m_lastTime = time;
    dt = std::clamp(dt, 0.001f, 0.1f);

    int targetCount = static_cast<int>(getParameter("count"));
    while (static_cast<int>(m_particles.size()) < targetCount) {
        Particle p;
        resetParticle(p, static_cast<float>(width), static_cast<float>(height));
        m_particles.push_back(p);
    }

    updateParticles(static_cast<float>(width), static_cast<float>(height), dt);
    std::memset(buffer, 0, width * height * channels * sizeof(float));
    renderParticles(buffer, width, height, channels);

    return PluginResult::ok();
}

void ParticleGeneratorPlugin::resetParticle(Particle& p, float width, float height) {
    float emitterX = static_cast<float>(getParameter("emitterX")) * width;
    float emitterY = static_cast<float>(getParameter("emitterY")) * height;
    float spread = static_cast<float>(getParameter("spread")) * 3.14159f / 180.0f;
    float speed = static_cast<float>(getParameter("speed"));
    float maxLife = static_cast<float>(getParameter("lifetime"));

    std::uniform_real_distribution<float> angleDist(-spread / 2.0f, spread / 2.0f);
    std::uniform_real_distribution<float> speedDist(speed * 0.5f, speed * 1.5f);
    std::uniform_real_distribution<float> lifeDist(maxLife * 0.5f, maxLife);
    std::uniform_real_distribution<float> sizeDist(0.5f, 1.5f);
    std::uniform_real_distribution<float> posDist(-10.0f, 10.0f);

    float angle = angleDist(m_rng);
    float spd = speedDist(m_rng);

    p.x = emitterX + posDist(m_rng);
    p.y = emitterY + posDist(m_rng);
    p.vx = std::cos(angle) * spd;
    p.vy = std::sin(angle) * spd;
    p.life = lifeDist(m_rng);
    p.maxLife = p.life;
    p.size = static_cast<float>(getParameter("size")) * sizeDist(m_rng);
    p.r = static_cast<float>(getParameter("colorR"));
    p.g = static_cast<float>(getParameter("colorG"));
    p.b = static_cast<float>(getParameter("colorB"));
    p.a = 1.0f;
}

void ParticleGeneratorPlugin::updateParticles(float width, float height, float dt) {
    float gravity = static_cast<float>(getParameter("gravity"));
    float fadeOut = static_cast<float>(getParameter("fadeOut"));
    float turbulence = static_cast<float>(getParameter("turbulence"));
    float windX = static_cast<float>(getParameter("windX"));
    float windY = static_cast<float>(getParameter("windY"));
    float sizeDecay = static_cast<float>(getParameter("sizeDecay"));

    std::uniform_real_distribution<float> turbDist(-turbulence, turbulence);

    for (auto it = m_particles.begin(); it != m_particles.end();) {
        it->life -= dt;
        if (it->life <= 0.0f) {
            resetParticle(*it, width, height);
            ++it;
            continue;
        }

        it->vx += (turbDist(m_rng) + windX) * dt;
        it->vy += (gravity + windY + turbDist(m_rng)) * dt;
        it->x += it->vx * dt;
        it->y += it->vy * dt;

        float lifeRatio = it->life / it->maxLife;
        if (fadeOut > 0.0f) {
            it->a = std::clamp(lifeRatio, 0.0f, 1.0f) * fadeOut + (1.0f - fadeOut);
        }
        if (sizeDecay > 0.0f) {
            it->size *= (1.0f - sizeDecay * dt);
        }

        if (it->x < -50 || it->x > width + 50 || it->y < -50 || it->y > height + 50) {
            resetParticle(*it, width, height);
        }
        ++it;
    }
}

void ParticleGeneratorPlugin::renderParticles(float* buffer, int width, int height, int channels) {
    for (const auto& p : m_particles) {
        int px = static_cast<int>(p.x);
        int py = static_cast<int>(p.y);
        int radius = std::max(1, static_cast<int>(p.size));

        for (int dy = -radius; dy <= radius; ++dy) {
            for (int dx = -radius; dx <= radius; ++dx) {
                int sx = px + dx;
                int sy = py + dy;
                if (sx < 0 || sx >= width || sy < 0 || sy >= height) continue;

                float dist = std::sqrt(static_cast<float>(dx * dx + dy * dy));
                if (dist > p.size) continue;

                float alpha = (1.0f - dist / p.size) * p.a;
                int idx = (sy * width + sx) * channels;

                if (channels >= 3) {
                    buffer[idx] = std::clamp(buffer[idx] + p.r * alpha, 0.0f, 1.0f);
                    buffer[idx + 1] = std::clamp(buffer[idx + 1] + p.g * alpha, 0.0f, 1.0f);
                    buffer[idx + 2] = std::clamp(buffer[idx + 2] + p.b * alpha, 0.0f, 1.0f);
                }
                if (channels >= 4) {
                    buffer[idx + 3] = std::clamp(buffer[idx + 3] + alpha, 0.0f, 1.0f);
                }
            }
        }
    }
}

void ParticleGeneratorPlugin::applyPreset(ParticlePreset preset) {
    switch (preset) {
        case ParticlePreset::Fire:
            setParameter("colorR", 1.0);
            setParameter("colorG", 0.3);
            setParameter("colorB", 0.0);
            setParameter("speed", 80.0);
            setParameter("gravity", -60.0);
            setParameter("spread", 25.0);
            setParameter("lifetime", 1.5);
            setParameter("size", 5.0);
            setParameter("count", 800);
            break;
        case ParticlePreset::Smoke:
            setParameter("colorR", 0.4);
            setParameter("colorG", 0.4);
            setParameter("colorB", 0.4);
            setParameter("speed", 20.0);
            setParameter("gravity", -30.0);
            setParameter("spread", 15.0);
            setParameter("lifetime", 4.0);
            setParameter("size", 15.0);
            setParameter("count", 300);
            setParameter("sizeDecay", 0.0);
            break;
        case ParticlePreset::Snow:
            setParameter("colorR", 0.9);
            setParameter("colorG", 0.95);
            setParameter("colorB", 1.0);
            setParameter("speed", 10.0);
            setParameter("gravity", 15.0);
            setParameter("spread", 180.0);
            setParameter("lifetime", 6.0);
            setParameter("size", 2.0);
            setParameter("count", 1000);
            setParameter("turbulence", 20.0);
            break;
        case ParticlePreset::Rain:
            setParameter("colorR", 0.6);
            setParameter("colorG", 0.7);
            setParameter("colorB", 1.0);
            setParameter("speed", 300.0);
            setParameter("gravity", 100.0);
            setParameter("spread", 10.0);
            setParameter("lifetime", 1.0);
            setParameter("size", 1.0);
            setParameter("count", 2000);
            break;
        case ParticlePreset::Sparkles:
            setParameter("colorR", 1.0);
            setParameter("colorG", 1.0);
            setParameter("colorB", 0.8);
            setParameter("speed", 40.0);
            setParameter("gravity", 10.0);
            setParameter("spread", 360.0);
            setParameter("lifetime", 2.0);
            setParameter("size", 2.0);
            setParameter("count", 400);
            setParameter("fadeOut", 1.0);
            break;
        case ParticlePreset::Custom:
            break;
    }
}

float ParticleGeneratorPlugin::clampf(float v) const {
    return std::clamp(v, 0.0f, 1.0f);
}

} // namespace FreeEffect
