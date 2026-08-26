#pragma once
#include "renderer.h"
#include <string>
#include <vector>
#include <memory>
#include <atomic>

namespace FreeEffect {

enum class GPUBackend { None, OpenGL, Vulkan, Metal, DirectX };

struct GPUDeviceInfo {
    std::string name;
    std::string vendor;
    size_t vramMB = 0;
    int computeUnits = 0;
    bool supportsCompute = false;
    bool supportsFloat16 = false;
};

class GPURenderer {
public:
    static GPURenderer& instance();

    bool initialize();
    void shutdown();
    bool isAvailable() const;
    GPUBackend getBackend() const;
    GPUDeviceInfo getDeviceInfo() const;

    void gaussianBlurGPU(const PixelBuffer& src, PixelBuffer& dst, int radius);
    void boxBlurGPU(const PixelBuffer& src, PixelBuffer& dst, int radius);
    void directionalBlurGPU(const PixelBuffer& src, PixelBuffer& dst, double angle, int radius);
    void lensBlurGPU(const PixelBuffer& src, PixelBuffer& dst, double focalDistance, double aperture);
    void sharpenGPU(const PixelBuffer& src, PixelBuffer& dst, double amount);

    void levelsGPU(const PixelBuffer& src, PixelBuffer& dst, double inBlack, double inWhite, double gamma, double outBlack, double outWhite);
    void curvesGPU(const PixelBuffer& src, PixelBuffer& dst, const float curve[256]);
    void hueSaturationGPU(const PixelBuffer& src, PixelBuffer& dst, double hue, double saturation, double lightness);
    void exposureGPU(const PixelBuffer& src, PixelBuffer& dst, double stops);
    void colorBalanceGPU(const PixelBuffer& src, PixelBuffer& dst, double shadowsR, double shadowsG, double shadowsB, double midR, double midG, double midB, double highR, double highG, double highB);

    void displacementMapGPU(const PixelBuffer& src, const PixelBuffer& map, PixelBuffer& dst, double scaleX, double scaleY);
    void waveWarpGPU(const PixelBuffer& src, PixelBuffer& dst, int waveType, double amplitude, double wavelength, double speed, double time);
    void turbulentDisplaceGPU(const PixelBuffer& src, PixelBuffer& dst, double amount, double scale, int complexity, double time);
    void bulgeGPU(const PixelBuffer& src, PixelBuffer& dst, double centerX, double centerY, double radius, double amount);
    void twirlGPU(const PixelBuffer& src, PixelBuffer& dst, double centerX, double centerY, double angle, double radius);

    void fractalNoiseGPU(PixelBuffer& dst, double scale, int octaves, double lacunarity, double gain, double time);
    void cloudGPU(PixelBuffer& dst, double scale, double time);
    void checkerboardGPU(PixelBuffer& dst, int size, const Color& color1, const Color& color2);
    void lensFlareGPU(PixelBuffer& dst, double centerX, double centerY, double intensity, double size);

    void glowGPU(const PixelBuffer& src, PixelBuffer& dst, double threshold, double radius, double intensity);
    void dropShadowGPU(const PixelBuffer& src, PixelBuffer& dst, double angle, double distance, double softness, const Color& color);
    void embossGPU(const PixelBuffer& src, PixelBuffer& dst, double angle, double depth);
    void findEdgesGPU(const PixelBuffer& src, PixelBuffer& dst);
    void posterizeGPU(const PixelBuffer& src, PixelBuffer& dst, int levels);
    void thresholdGPU(const PixelBuffer& src, PixelBuffer& dst, double threshold);
    void noiseGPU(PixelBuffer& dst, double amount, bool monochromatic, double time);
    void mosaicGPU(const PixelBuffer& src, PixelBuffer& dst, int blockWidth, int blockHeight);

    void keylightGPU(const PixelBuffer& src, PixelBuffer& dst, const Color& keyColor, double tolerance, double softness, double spillSuppression);

    void radialBlurGPU(const PixelBuffer& src, PixelBuffer& dst, double centerX, double centerY, int amount, int type);
    void ccRadialBlurGPU(const PixelBuffer& src, PixelBuffer& dst, double centerX, double centerY, int amount);
    void vectorBlurGPU(const PixelBuffer& src, PixelBuffer& dst, double amount, double angle);

    void linearWipeGPU(const PixelBuffer& src, PixelBuffer& dst, double angle, double completion, double feather);
    void radialWipeGPU(const PixelBuffer& src, PixelBuffer& dst, double completion, double center, double startAngle, double feather);

    void setMaxGPUMemoryMB(size_t mb);
    size_t getUsedGPUMemory() const;
    bool isEffectGPUAccelerated(const std::string& effectName) const;

private:
    GPURenderer() = default;

    static inline float clampf(float v, float lo, float hi) {
        return v < lo ? lo : (v > hi ? hi : v);
    }

    bool m_initialized = false;
    GPUBackend m_backend = GPUBackend::None;
    GPUDeviceInfo m_info;
    std::atomic<size_t> m_usedGPUMemory{0};
    size_t m_maxGPUMemoryMB = 512;
};

} // namespace FreeEffect
