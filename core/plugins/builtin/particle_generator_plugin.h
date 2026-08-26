#pragma once

#include "../plugin_interface.h"
#include <cmath>
#include <random>
#include <string>
#include <vector>

namespace FreeEffect {

struct Particle {
    float x = 0.0f;
    float y = 0.0f;
    float vx = 0.0f;
    float vy = 0.0f;
    float size = 1.0f;
    float life = 1.0f;
    float maxLife = 1.0f;
    float r = 1.0f;
    float g = 1.0f;
    float b = 1.0f;
    float a = 1.0f;
};

enum class ParticlePreset {
    Fire,
    Smoke,
    Snow,
    Rain,
    Sparkles,
    Custom
};

class ParticleGeneratorPlugin : public PluginInterface {
public:
    ParticleGeneratorPlugin();
    ~ParticleGeneratorPlugin() override;

    PluginResult initialize() override;
    PluginResult shutdown() override;

    const char* getName() const override;
    const char* getVersion() const override;
    const char* getDescription() const override;
    const char* getAuthor() const override;
    PluginAPIVersion getAPIVersion() const override;
    PluginType getType() const override;

    uint32_t getParameterCount() const override;
    PluginParameter getParameter(uint32_t index) const override;
    PluginResult setParameter(const std::string& name, double value) override;
    double getParameter(const std::string& name) const override;

    bool needsProcessing() const override;
    PluginResult process(float* buffer, int width, int height, int channels, double time) override;

private:
    void resetParticle(Particle& p, float width, float height);
    void updateParticles(float width, float height, float dt);
    void renderParticles(float* buffer, int width, int height, int channels);
    void applyPreset(ParticlePreset preset);
    float clampf(float v) const;

    std::vector<Particle> m_particles;
    std::mt19937 m_rng;
    std::vector<PluginParameter> m_parameters;
    bool m_initialized = false;
    double m_lastTime = -1.0;
};

} // namespace FreeEffect
