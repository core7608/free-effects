#include "opengl_renderer.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <functional>

namespace FreeEffect {

OpenGLRenderer::OpenGLRenderer() {}

OpenGLRenderer::~OpenGLRenderer() {
    shutdown();
}

bool OpenGLRenderer::initialize() {
    if (m_initialized) return true;

    m_hasGPU = false;

#ifdef HAS_OPENGL
    m_hasGPU = true;
#endif

    m_initialized = true;
    return true;
}

void OpenGLRenderer::shutdown() {
    m_initialized = false;
}

PixelBuffer OpenGLRenderer::renderFrame(const Composition& comp, double time) {
    Renderer renderer;
    renderer.setResolution(comp.getResolution().width, comp.getResolution().height);
    return renderer.renderFrame(comp, time);
}

unsigned int OpenGLRenderer::compileShader(const std::string& vertex, const std::string& fragment) {
#ifdef HAS_OPENGL
    if (!m_hasGPU) return 0;

    auto compile = [](unsigned int type, const std::string& src) -> unsigned int {
        unsigned int id = 0;
        const char* source = src.c_str();
        glShaderSource(id, 1, &source, nullptr);
        glCompileShader(id);
        int result = 0;
        glGetShaderiv(id, GL_COMPILE_STATUS, &result);
        if (!result) {
            glDeleteShader(id);
            return 0;
        }
        return id;
    };

    unsigned int vs = compile(GL_VERTEX_SHADER, vertex);
    if (!vs) return 0;

    unsigned int fs = compile(GL_FRAGMENT_SHADER, fragment);
    if (!fs) { glDeleteShader(vs); return 0; }

    unsigned int program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    int linked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked) {
        glDeleteProgram(program);
        glDeleteShader(vs);
        glDeleteShader(fs);
        return 0;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
    return program;
#else
    (void)vertex;
    (void)fragment;
    return 0;
#endif
}

void OpenGLRenderer::deleteShader(unsigned int programId) {
    if (programId == 0) return;
#ifdef HAS_OPENGL
    glDeleteProgram(programId);
#endif
}

float OpenGLRenderer::edgeFunction(float a[2], float b[2], float c[2]) {
    return (c[0] - a[0]) * (b[1] - a[1]) - (c[1] - a[1]) * (b[0] - a[0]);
}

void OpenGLRenderer::fillTriangle(float x0, float y0, float x1, float y1, float x2, float y2,
                                   float c0[4], float c1[4], float c2[4], PixelBuffer& buffer) {
    int minX = std::max(0, static_cast<int>(std::floor(std::min({x0, x1, x2}))));
    int minY = std::max(0, static_cast<int>(std::floor(std::min({y0, y1, y2}))));
    int maxX = std::min(buffer.width - 1, static_cast<int>(std::ceil(std::max({x0, x1, x2}))));
    int maxY = std::min(buffer.height - 1, static_cast<int>(std::ceil(std::max({y0, y1, y2}))));

    float a0[2] = {x1, y1};
    float a1[2] = {x2, y2};
    float a2[2] = {x0, y0};
    float b0[2] = {x2, y2};
    float b1[2] = {x0, y0};
    float b2[2] = {x1, y1};
    float c0v[2] = {x0, y0};
    float c1v[2] = {x1, y1};
    float c2v[2] = {x2, y2};

    float area = edgeFunction(a2, a0, a1);
    if (std::abs(area) < 1e-6f) return;

    for (int y = minY; y <= maxY; ++y) {
        for (int x = minX; x <= maxX; ++x) {
            float p[2] = {static_cast<float>(x) + 0.5f, static_cast<float>(y) + 0.5f};

            float w0 = edgeFunction(a0, a1, p) / area;
            float w1 = edgeFunction(a1, a2, p) / area;
            float w2 = edgeFunction(a2, a0, p) / area;

            if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                float r = w0 * c0[0] + w1 * c1[0] + w2 * c2[0];
                float g = w0 * c0[1] + w1 * c1[1] + w2 * c2[1];
                float b = w0 * c0[2] + w1 * c1[2] + w2 * c2[2];
                float a = w0 * c0[3] + w1 * c1[3] + w2 * c2[3];

                uint8_t* pixel = buffer.pixelAt(x, y);
                pixel[0] = static_cast<uint8_t>(std::clamp(r * 255.0f, 0.0f, 255.0f));
                pixel[1] = static_cast<uint8_t>(std::clamp(g * 255.0f, 0.0f, 255.0f));
                pixel[2] = static_cast<uint8_t>(std::clamp(b * 255.0f, 0.0f, 255.0f));
                pixel[3] = static_cast<uint8_t>(std::clamp(a * 255.0f, 0.0f, 255.0f));
            }
        }
    }
}

void OpenGLRenderer::rasterizeTriangle(float v0[3], float v1[3], float v2[3],
                                        float c0[4], float c1[4], float c2[4],
                                        PixelBuffer& buffer, float viewProj[16]) {
    auto projectToScreen = [&](float v[3], float& sx, float& sy) {
        float x = viewProj[0] * v[0] + viewProj[4] * v[1] + viewProj[8] * v[2] + viewProj[12];
        float y = viewProj[1] * v[0] + viewProj[5] * v[1] + viewProj[9] * v[2] + viewProj[13];
        float w = viewProj[3] * v[0] + viewProj[7] * v[1] + viewProj[11] * v[2] + viewProj[15];
        if (std::abs(w) < 1e-6f) { sx = 0; sy = 0; return; }
        float ndcX = x / w;
        float ndcY = y / w;
        sx = (ndcX + 1.0f) * 0.5f * static_cast<float>(buffer.width);
        sy = (1.0f - ndcY) * 0.5f * static_cast<float>(buffer.height);
    };

    float sx0, sy0, sx1, sy1, sx2, sy2;
    projectToScreen(v0, sx0, sy0);
    projectToScreen(v1, sx1, sy1);
    projectToScreen(v2, sx2, sy2);

    fillTriangle(sx0, sy0, sx1, sy1, sx2, sy2, c0, c1, c2, buffer);
}

PixelBuffer OpenGLRenderer::applyGaussianBlur(const PixelBuffer& input, int radius) {
    if (radius <= 0) return input;

    int w = input.width;
    int h = input.height;
    PixelBuffer temp;
    temp.resize(w, h);
    PixelBuffer output;
    output.resize(w, h);

    int kernelSize = radius * 2 + 1;
    std::vector<float> kernel(kernelSize);
    float sigma = static_cast<float>(radius) / 3.0f;
    float sum = 0.0f;
    for (int i = 0; i < kernelSize; ++i) {
        float x = static_cast<float>(i - radius);
        kernel[i] = std::exp(-(x * x) / (2.0f * sigma * sigma));
        sum += kernel[i];
    }
    for (int i = 0; i < kernelSize; ++i) {
        kernel[i] /= sum;
    }

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            float r = 0, g = 0, b = 0, a = 0;
            for (int k = -radius; k <= radius; ++k) {
                int sx = std::clamp(x + k, 0, w - 1);
                const uint8_t* p = input.pixelAt(sx, y);
                float weight = kernel[k + radius];
                r += p[0] * weight;
                g += p[1] * weight;
                b += p[2] * weight;
                a += p[3] * weight;
            }
            uint8_t* dst = temp.pixelAt(x, y);
            dst[0] = static_cast<uint8_t>(std::clamp(r, 0.0f, 255.0f));
            dst[1] = static_cast<uint8_t>(std::clamp(g, 0.0f, 255.0f));
            dst[2] = static_cast<uint8_t>(std::clamp(b, 0.0f, 255.0f));
            dst[3] = static_cast<uint8_t>(std::clamp(a, 0.0f, 255.0f));
        }
    }

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            float r = 0, g = 0, b = 0, a = 0;
            for (int k = -radius; k <= radius; ++k) {
                int sy = std::clamp(y + k, 0, h - 1);
                const uint8_t* p = temp.pixelAt(x, sy);
                float weight = kernel[k + radius];
                r += p[0] * weight;
                g += p[1] * weight;
                b += p[2] * weight;
                a += p[3] * weight;
            }
            uint8_t* dst = output.pixelAt(x, y);
            dst[0] = static_cast<uint8_t>(std::clamp(r, 0.0f, 255.0f));
            dst[1] = static_cast<uint8_t>(std::clamp(g, 0.0f, 255.0f));
            dst[2] = static_cast<uint8_t>(std::clamp(b, 0.0f, 255.0f));
            dst[3] = static_cast<uint8_t>(std::clamp(a, 0.0f, 255.0f));
        }
    }

    return output;
}

void OpenGLRenderer::applyGammaCorrection(PixelBuffer& buffer, double gamma) {
    if (gamma <= 0.0 || std::abs(gamma - 1.0) < 1e-6) return;

    double invGamma = 1.0 / gamma;
    int pixelCount = buffer.width * buffer.height;

    for (int p = 0; p < pixelCount; ++p) {
        int offset = p * 4;
        double r = buffer.data[offset] / 255.0;
        double g = buffer.data[offset + 1] / 255.0;
        double b = buffer.data[offset + 2] / 255.0;

        r = std::pow(r, invGamma);
        g = std::pow(g, invGamma);
        b = std::pow(b, invGamma);

        buffer.data[offset]     = static_cast<uint8_t>(std::clamp(r * 255.0, 0.0, 255.0));
        buffer.data[offset + 1] = static_cast<uint8_t>(std::clamp(g * 255.0, 0.0, 255.0));
        buffer.data[offset + 2] = static_cast<uint8_t>(std::clamp(b * 255.0, 0.0, 255.0));
    }
}

std::array<std::array<int, 256>, 3> OpenGLRenderer::computeHistogram(const PixelBuffer& buffer) {
    std::array<std::array<int, 256>, 3> hist{};
    int pixelCount = buffer.width * buffer.height;

    for (int p = 0; p < pixelCount; ++p) {
        int offset = p * 4;
        hist[0][buffer.data[offset]]++;
        hist[1][buffer.data[offset + 1]]++;
        hist[2][buffer.data[offset + 2]]++;
    }

    return hist;
}

void OpenGLRenderer::applyLUT(PixelBuffer& buffer, const uint8_t lut[256][256][256]) {
    int pixelCount = buffer.width * buffer.height;

    for (int p = 0; p < pixelCount; ++p) {
        int offset = p * 4;
        uint8_t r = buffer.data[offset];
        uint8_t g = buffer.data[offset + 1];
        uint8_t b = buffer.data[offset + 2];

        uint8_t mappedR = lut[r][g][b];
        uint8_t mappedG = lut[g][b][r];
        uint8_t mappedB = lut[b][r][g];

        buffer.data[offset]     = mappedR;
        buffer.data[offset + 1] = mappedG;
        buffer.data[offset + 2] = mappedB;
    }
}

} // namespace FreeEffect
