#pragma once

#include "../plugin_interface.h"
#include <cmath>
#include <string>
#include <vector>

namespace FreeEffect {

enum class TextAnimPreset {
    None,
    Typewriter,
    FadeInPerChar,
    Bounce,
    ScaleUp,
    RotateIn,
    BlurIn,
    Wave
};

struct TextChar {
    float x = 0.0f;
    float y = 0.0f;
    float rotation = 0.0f;
    float scaleX = 1.0f;
    float scaleY = 1.0f;
    float opacity = 1.0f;
    float offsetX = 0.0f;
    float offsetY = 0.0f;
};

class TextAnimatorPlugin : public PluginInterface {
public:
    TextAnimatorPlugin();
    ~TextAnimatorPlugin() override;

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
    void applyTypewriter(float* buffer, int width, int height, int channels, double time);
    void applyFadeInPerChar(float* buffer, int width, int height, int channels, double time);
    void applyBounce(float* buffer, int width, int height, int channels, double time);
    void applyScaleUp(float* buffer, int width, int height, int channels, double time);
    void applyRotateIn(float* buffer, int width, int height, int channels, double time);
    void applyBlurIn(float* buffer, int width, int height, int channels, double time);
    void applyWave(float* buffer, int width, int height, int channels, double time);
    void renderCharacter(float* buffer, int width, int height, int channels, int cx, int cy, float size, float r, float g, float b, float a, float rotation);
    float clampf(float v) const;

    std::vector<PluginParameter> m_parameters;
    bool m_initialized = false;
};

} // namespace FreeEffect
