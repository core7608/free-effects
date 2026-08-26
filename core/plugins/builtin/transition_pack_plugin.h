#pragma once

#include "../plugin_interface.h"
#include <cmath>
#include <string>
#include <vector>

namespace FreeEffect {

enum class TransitionPreset {
    Fade,
    WipeLeft,
    WipeRight,
    WipeUp,
    WipeDown,
    ZoomIn,
    ZoomOut,
    SlideLeft,
    SlideRight,
    SlideUp,
    SlideDown,
    Dissolve,
    Blur,
    Glow,
    Morph,
    ClockWipe,
    DiamondWipe,
    CrossZoom,
    PushLeft,
    PushRight,
    LumaFade,
    RadialWipe
};

struct TransitionFrame {
    float progress = 0.0f;
    int width = 0;
    int height = 0;
};

class TransitionPackPlugin : public PluginInterface {
public:
    TransitionPackPlugin();
    ~TransitionPackPlugin() override;

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
    void applyFade(float* buffer, int width, int height, int channels, float progress);
    void applyWipe(float* buffer, int width, int height, int channels, float progress, int direction);
    void applyZoom(float* buffer, int width, int height, int channels, float progress, bool zoomIn);
    void applySlide(float* buffer, int width, int height, int channels, float progress, int direction);
    void applyDissolve(float* buffer, int width, int height, int channels, float progress);
    void applyBlur(float* buffer, int width, int height, int channels, float progress);
    void applyGlow(float* buffer, int width, int height, int channels, float progress);
    void applyClockWipe(float* buffer, int width, int height, int channels, float progress);
    void applyDiamondWipe(float* buffer, int width, int height, int channels, float progress);
    void applyCrossZoom(float* buffer, int width, int height, int channels, float progress);
    void applyPush(float* buffer, int width, int height, int channels, float progress, int direction);
    void applyLumaFade(float* buffer, int width, int height, int channels, float progress);
    void applyRadialWipe(float* buffer, int width, int height, int channels, float progress);
    void applyMorph(float* buffer, int width, int height, int channels, float progress);
    void applyBoxBlur(float* buffer, int width, int height, int channels, int radius);
    float clampf(float v) const;

    std::vector<PluginParameter> m_parameters;
    bool m_initialized = false;
};

} // namespace FreeEffect
