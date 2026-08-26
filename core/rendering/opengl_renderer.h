#pragma once

#include "../timeline/composition.h"
#include "renderer.h"
#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace FreeEffect {

struct GLEffectShader {
    std::string vertexShader;
    std::string fragmentShader;
    unsigned int programId = 0;
    bool compiled = false;
};

class OpenGLRenderer {
public:
    OpenGLRenderer();
    ~OpenGLRenderer();

    bool initialize();
    void shutdown();
    bool isInitialized() const { return m_initialized; }

    bool hasGPU() const { return m_hasGPU; }

    PixelBuffer renderFrame(const Composition& comp, double time);

    unsigned int compileShader(const std::string& vertex, const std::string& fragment);
    void deleteShader(unsigned int programId);

    void rasterizeTriangle(float v0[3], float v1[3], float v2[3],
                           float c0[4], float c1[4], float c2[4],
                           PixelBuffer& buffer, float viewProj[16]);

    PixelBuffer applyGaussianBlur(const PixelBuffer& input, int radius);

    void applyGammaCorrection(PixelBuffer& buffer, double gamma);

    std::array<std::array<int, 256>, 3> computeHistogram(const PixelBuffer& buffer);

    void applyLUT(PixelBuffer& buffer, const uint8_t lut[256][256][256]);

private:
    bool m_initialized = false;
    bool m_hasGPU = false;

    void fillTriangle(float x0, float y0, float x1, float y1, float x2, float y2,
                      float c0[4], float c1[4], float c2[4], PixelBuffer& buffer);
    float edgeFunction(float a[2], float b[2], float c[2]);
};

} // namespace FreeEffect
