#pragma once

#include "../plugin_interface.h"
#include <cmath>
#include <string>
#include <vector>

namespace FreeEffect {

class LensEffectsPlugin : public PluginInterface {
public:
    LensEffectsPlugin();
    ~LensEffectsPlugin() override;

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
    void applyDistortion(float* buffer, int width, int height, int channels);
    void applyChromaticAberration(float* buffer, int srcWidth, int srcHeight, int channels);
    void applyVignette(float* buffer, int width, int height, int channels);
    void applyFisheye(float* buffer, int width, int height, int channels);
    void applyBarrelDistortion(float* buffer, int width, int height, int channels);
    float clampf(float v) const;

    std::vector<PluginParameter> m_parameters;
    bool m_initialized = false;
};

} // namespace FreeEffect
