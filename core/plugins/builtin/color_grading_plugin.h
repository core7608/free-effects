#pragma once

#include "../plugin_interface.h"
#include <cmath>
#include <string>
#include <vector>

namespace FreeEffect {

class ColorGradingPlugin : public PluginInterface {
public:
    ColorGradingPlugin();
    ~ColorGradingPlugin() override;

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
    void applyLift(float& r, float& g, float& b) const;
    void applyGamma(float& r, float& g, float& b) const;
    void applyGain(float& r, float& g, float& b) const;
    void applySaturation(float& r, float& g, float& b) const;
    void applyTemperature(float& r, float& g, float& b) const;
    void applyContrast(float& r, float& g, float& b) const;
    void applyHighlights(float& r, float& g, float& b) const;
    void applyShadows(float& r, float& g, float& b) const;
    float luminance(float r, float g, float b) const;
    float clampf(float v) const;

    std::vector<PluginParameter> m_parameters;
    bool m_initialized = false;
};

} // namespace FreeEffect
