#include "gpu_renderer.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <thread>
#include <vector>
#include <numeric>
#include <random>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace FreeEffect {

GPURenderer& GPURenderer::instance() {
    static GPURenderer inst;
    return inst;
}

bool GPURenderer::initialize() {
    if (m_initialized) return true;

#ifdef HAS_OPENGL
    m_backend = GPUBackend::OpenGL;
    m_info.name = "OpenGL GPU";
    m_info.vendor = "OpenGL";
    m_info.supportsCompute = false;
    m_info.supportsFloat16 = false;
    m_info.vramMB = 256;
    m_info.computeUnits = 1;
#else
    m_backend = GPUBackend::None;
    m_info.name = "CPU Fallback";
    m_info.vendor = "Software";
    m_info.vramMB = 0;
    m_info.computeUnits = static_cast<int>(std::thread::hardware_concurrency());
    m_info.supportsCompute = false;
    m_info.supportsFloat16 = false;
#endif

    m_initialized = true;
    return true;
}

void GPURenderer::shutdown() {
    m_initialized = false;
    m_backend = GPUBackend::None;
    m_usedGPUMemory.store(0);
}

bool GPURenderer::isAvailable() const {
    return m_initialized;
}

GPUBackend GPURenderer::getBackend() const {
    return m_backend;
}

GPUDeviceInfo GPURenderer::getDeviceInfo() const {
    return m_info;
}

void GPURenderer::setMaxGPUMemoryMB(size_t mb) {
    m_maxGPUMemoryMB = mb;
}

size_t GPURenderer::getUsedGPUMemory() const {
    return m_usedGPUMemory.load();
}

bool GPURenderer::isEffectGPUAccelerated(const std::string& effectName) const {
    if (m_backend == GPUBackend::None) return false;
    static const std::vector<std::string> gpuEffects = {
        "Gaussian Blur", "Box Blur", "Sharpen", "Levels", "Curves",
        "Hue/Saturation", "Exposure", "Glow", "Drop Shadow", "Find Edges",
        "Posterize", "Threshold", "Mosaic", "Emboss", "Displacement Map",
        "Wave Warp", "Bulge", "Twirl", "Fractal Noise", "Turbulent Displace"
    };
    for (const auto& e : gpuEffects) {
        if (e == effectName) return true;
    }
    return false;
}

void GPURenderer::computeKernel1D(const std::string& kernel, const PixelBuffer& src, PixelBuffer& dst, int param) {
    (void)kernel; (void)src; (void)dst; (void)param;
}

void GPURenderer::computeKernel2D(const std::string& kernel, const PixelBuffer& src, PixelBuffer& dst, int param) {
    (void)kernel; (void)src; (void)dst; (void)param;
}

// ─── Helper: parallel row range ──────────────────────────────────────
static void parallelRows(int height, std::function<void(int)> func) {
    int hw = static_cast<int>(std::thread::hardware_concurrency());
    int threads = std::max(1, std::min(hw, height));
    if (threads <= 1 || height < 64) {
        for (int y = 0; y < height; ++y) func(y);
        return;
    }
    std::vector<std::thread> pool;
    pool.reserve(threads);
    int rowsPerThread = height / threads;
    int extra = height % threads;
    int startRow = 0;
    for (int t = 0; t < threads; ++t) {
        int count = rowsPerThread + (t < extra ? 1 : 0);
        int s = startRow;
        int e = s + count;
        startRow = e;
        pool.emplace_back([s, e, &func]() {
            for (int y = s; y < e; ++y) func(y);
        });
    }
    for (auto& t : pool) t.join();
}

// ═══════════════════════════════════════════════════════════════════════
// BLUR EFFECTS
// ═══════════════════════════════════════════════════════════════════════

void GPURenderer::gaussianBlurGPU(const PixelBuffer& src, PixelBuffer& dst, int radius) {
    if (radius <= 0) { dst = src; return; }
    int w = src.width, h = src.height;
    dst.resize(w, h);

    int kernelSize = radius * 2 + 1;
    std::vector<float> kernel(kernelSize);
    float sigma = static_cast<float>(radius) / 3.0f;
    float sum = 0.0f;
    for (int i = 0; i < kernelSize; ++i) {
        float x = static_cast<float>(i - radius);
        kernel[i] = std::exp(-(x * x) / (2.0f * sigma * sigma));
        sum += kernel[i];
    }
    for (int i = 0; i < kernelSize; ++i) kernel[i] /= sum;

    PixelBuffer temp;
    temp.resize(w, h);

    parallelRows(h, [&](int y) {
        for (int x = 0; x < w; ++x) {
            float r = 0, g = 0, b = 0, a = 0;
            for (int k = -radius; k <= radius; ++k) {
                int sx = std::clamp(x + k, 0, w - 1);
                const uint8_t* p = src.pixelAt(sx, y);
                float wt = kernel[k + radius];
                r += p[0] * wt;
                g += p[1] * wt;
                b += p[2] * wt;
                a += p[3] * wt;
            }
            uint8_t* d = temp.pixelAt(x, y);
            d[0] = static_cast<uint8_t>(std::clamp(r, 0.0f, 255.0f));
            d[1] = static_cast<uint8_t>(std::clamp(g, 0.0f, 255.0f));
            d[2] = static_cast<uint8_t>(std::clamp(b, 0.0f, 255.0f));
            d[3] = static_cast<uint8_t>(std::clamp(a, 0.0f, 255.0f));
        }
    });

    parallelRows(h, [&](int y) {
        for (int x = 0; x < w; ++x) {
            float r = 0, g = 0, b = 0, a = 0;
            for (int k = -radius; k <= radius; ++k) {
                int sy = std::clamp(y + k, 0, h - 1);
                const uint8_t* p = temp.pixelAt(x, sy);
                float wt = kernel[k + radius];
                r += p[0] * wt;
                g += p[1] * wt;
                b += p[2] * wt;
                a += p[3] * wt;
            }
            uint8_t* d = dst.pixelAt(x, y);
            d[0] = static_cast<uint8_t>(std::clamp(r, 0.0f, 255.0f));
            d[1] = static_cast<uint8_t>(std::clamp(g, 0.0f, 255.0f));
            d[2] = static_cast<uint8_t>(std::clamp(b, 0.0f, 255.0f));
            d[3] = static_cast<uint8_t>(std::clamp(a, 0.0f, 255.0f));
        }
    });
}

void GPURenderer::boxBlurGPU(const PixelBuffer& src, PixelBuffer& dst, int radius) {
    if (radius <= 0) { dst = src; return; }
    int w = src.width, h = src.height;
    dst.resize(w, h);

    PixelBuffer temp;
    temp.resize(w, h);
    float invSize = 1.0f / static_cast<float>(2 * radius + 1);

    parallelRows(h, [&](int y) {
        for (int x = 0; x < w; ++x) {
            float r = 0, g = 0, b = 0, a = 0;
            for (int k = -radius; k <= radius; ++k) {
                int sx = std::clamp(x + k, 0, w - 1);
                const uint8_t* p = src.pixelAt(sx, y);
                r += p[0]; g += p[1]; b += p[2]; a += p[3];
            }
            uint8_t* d = temp.pixelAt(x, y);
            d[0] = static_cast<uint8_t>(std::clamp(r * invSize, 0.0f, 255.0f));
            d[1] = static_cast<uint8_t>(std::clamp(g * invSize, 0.0f, 255.0f));
            d[2] = static_cast<uint8_t>(std::clamp(b * invSize, 0.0f, 255.0f));
            d[3] = static_cast<uint8_t>(std::clamp(a * invSize, 0.0f, 255.0f));
        }
    });

    parallelRows(h, [&](int y) {
        for (int x = 0; x < w; ++x) {
            float r = 0, g = 0, b = 0, a = 0;
            for (int k = -radius; k <= radius; ++k) {
                int sy = std::clamp(y + k, 0, h - 1);
                const uint8_t* p = temp.pixelAt(x, sy);
                r += p[0]; g += p[1]; b += p[2]; a += p[3];
            }
            uint8_t* d = dst.pixelAt(x, y);
            d[0] = static_cast<uint8_t>(std::clamp(r * invSize, 0.0f, 255.0f));
            d[1] = static_cast<uint8_t>(std::clamp(g * invSize, 0.0f, 255.0f));
            d[2] = static_cast<uint8_t>(std::clamp(b * invSize, 0.0f, 255.0f));
            d[3] = static_cast<uint8_t>(std::clamp(a * invSize, 0.0f, 255.0f));
        }
    });
}

void GPURenderer::directionalBlurGPU(const PixelBuffer& src, PixelBuffer& dst, double angle, int radius) {
    if (radius <= 0) { dst = src; return; }
    int w = src.width, h = src.height;
    dst.resize(w, h);
    double rad = angle * M_PI / 180.0;
    double dx = std::cos(rad);
    double dy = std::sin(rad);

    parallelRows(h, [&](int y) {
        for (int x = 0; x < w; ++x) {
            float r = 0, g = 0, b = 0, a = 0;
            int count = 0;
            for (int k = -radius; k <= radius; ++k) {
                int sx = std::clamp(static_cast<int>(x + dx * k), 0, w - 1);
                int sy = std::clamp(static_cast<int>(y + dy * k), 0, h - 1);
                const uint8_t* p = src.pixelAt(sx, sy);
                r += p[0]; g += p[1]; b += p[2]; a += p[3];
                count++;
            }
            float inv = 1.0f / static_cast<float>(count);
            uint8_t* d = dst.pixelAt(x, y);
            d[0] = static_cast<uint8_t>(std::clamp(r * inv, 0.0f, 255.0f));
            d[1] = static_cast<uint8_t>(std::clamp(g * inv, 0.0f, 255.0f));
            d[2] = static_cast<uint8_t>(std::clamp(b * inv, 0.0f, 255.0f));
            d[3] = static_cast<uint8_t>(std::clamp(a * inv, 0.0f, 255.0f));
        }
    });
}

void GPURenderer::lensBlurGPU(const PixelBuffer& src, PixelBuffer& dst, double focalDistance, double aperture) {
    int radius = std::max(1, static_cast<int>(aperture * 5));
    gaussianBlurGPU(src, dst, radius);
    if (focalDistance > 0.0) {
        int w = dst.width, h = dst.height;
        float focalDist = static_cast<float>(focalDistance);
        parallelRows(h, [&](int y) {
            for (int x = 0; x < w; ++x) {
                float dx = static_cast<float>(x) - w * 0.5f;
                float dy = static_cast<float>(y) - h * 0.5f;
                float dist = std::sqrt(dx * dx + dy * dy);
                float focus = std::abs(dist - focalDist) / focalDist;
                focus = std::clamp(focus, 0.0f, 1.0f);
                uint8_t* p = dst.pixelAt(x, y);
                const uint8_t* s = src.pixelAt(x, y);
                p[0] = static_cast<uint8_t>(std::clamp(
                    p[0] * focus + s[0] * (1.0f - focus), 0.0f, 255.0f));
                p[1] = static_cast<uint8_t>(std::clamp(
                    p[1] * focus + s[1] * (1.0f - focus), 0.0f, 255.0f));
                p[2] = static_cast<uint8_t>(std::clamp(
                    p[2] * focus + s[2] * (1.0f - focus), 0.0f, 255.0f));
            }
        });
    }
}

void GPURenderer::sharpenGPU(const PixelBuffer& src, PixelBuffer& dst, double amount) {
    int w = src.width, h = src.height;
    dst.resize(w, h);
    float amt = static_cast<float>(amount);

    parallelRows(h, [&](int y) {
        for (int x = 0; x < w; ++x) {
            const uint8_t* c = src.pixelAt(x, y);
            float cr = c[0], cg = c[1], cb = c[2];
            float sr = 0, sg = 0, sb = 0;
            int cnt = 0;
            for (int dy = -1; dy <= 1; ++dy) {
                for (int dx = -1; dx <= 1; ++dx) {
                    if (dx == 0 && dy == 0) continue;
                    int sx = std::clamp(x + dx, 0, w - 1);
                    int sy = std::clamp(y + dy, 0, h - 1);
                    const uint8_t* p = src.pixelAt(sx, sy);
                    sr += p[0]; sg += p[1]; sb += p[2];
                    cnt++;
                }
            }
            sr /= cnt; sg /= cnt; sb /= cnt;
            float r = cr + (cr - sr) * amt;
            float g = cg + (cg - sg) * amt;
            float b = cb + (cb - sb) * amt;
            uint8_t* d = dst.pixelAt(x, y);
            d[0] = static_cast<uint8_t>(std::clamp(r, 0.0f, 255.0f));
            d[1] = static_cast<uint8_t>(std::clamp(g, 0.0f, 255.0f));
            d[2] = static_cast<uint8_t>(std::clamp(b, 0.0f, 255.0f));
            d[3] = c[3];
        }
    });
}

// ═══════════════════════════════════════════════════════════════════════
// COLOR OPERATIONS
// ═══════════════════════════════════════════════════════════════════════

void GPURenderer::levelsGPU(const PixelBuffer& src, PixelBuffer& dst,
                            double inBlack, double inWhite, double gamma,
                            double outBlack, double outWhite) {
    int w = src.width, h = src.height;
    dst.resize(w, h);
    float inRange = std::max(1.0f, static_cast<float>(inWhite - inBlack));
    float outRange = static_cast<float>(outWhite - outBlack);
    float invGamma = (gamma > 0.0) ? static_cast<float>(1.0 / gamma) : 1.0f;

    uint8_t lut[256];
    for (int i = 0; i < 256; ++i) {
        float v = (static_cast<float>(i) - static_cast<float>(inBlack)) / inRange;
        v = std::clamp(v, 0.0f, 1.0f);
        v = std::pow(v, invGamma);
        v = v * outRange + static_cast<float>(outBlack);
        lut[i] = static_cast<uint8_t>(std::clamp(v * 255.0f, 0.0f, 255.0f));
    }

    parallelRows(h, [&](int y) {
        for (int x = 0; x < w; ++x) {
            const uint8_t* s = src.pixelAt(x, y);
            uint8_t* d = dst.pixelAt(x, y);
            d[0] = lut[s[0]];
            d[1] = lut[s[1]];
            d[2] = lut[s[2]];
            d[3] = s[3];
        }
    });
}

void GPURenderer::curvesGPU(const PixelBuffer& src, PixelBuffer& dst, const float curve[256]) {
    int w = src.width, h = src.height;
    dst.resize(w, h);

    uint8_t lut[256];
    for (int i = 0; i < 256; ++i) {
        lut[i] = static_cast<uint8_t>(std::clamp(curve[i] * 255.0f, 0.0f, 255.0f));
    }

    parallelRows(h, [&](int y) {
        for (int x = 0; x < w; ++x) {
            const uint8_t* s = src.pixelAt(x, y);
            uint8_t* d = dst.pixelAt(x, y);
            d[0] = lut[s[0]];
            d[1] = lut[s[1]];
            d[2] = lut[s[2]];
            d[3] = s[3];
        }
    });
}

void GPURenderer::hueSaturationGPU(const PixelBuffer& src, PixelBuffer& dst,
                                    double hue, double saturation, double lightness) {
    int w = src.width, h = src.height;
    dst.resize(w, h);
    float hShift = static_cast<float>(hue) / 360.0f;
    float sMul = 1.0f + static_cast<float>(saturation) / 100.0f;
    float lAdd = static_cast<float>(lightness) / 100.0f;

    auto rgbToHsl = [](float r, float g, float b, float& h, float& s, float& l) {
        float mx = std::max({r, g, b});
        float mn = std::min({r, g, b});
        l = (mx + mn) * 0.5f;
        if (mx == mn) { h = s = 0; return; }
        float d = mx - mn;
        s = l > 0.5f ? d / (2.0f - mx - mn) : d / (mx + mn);
        if (mx == r) h = (g - b) / d + (g < b ? 6.0f : 0.0f);
        else if (mx == g) h = (b - r) / d + 2.0f;
        else h = (r - g) / d + 4.0f;
        h /= 6.0f;
    };

    auto hslToRgb = [](float h, float s, float l, float& r, float& g, float& b) {
        if (s == 0) { r = g = b = l; return; }
        auto hue2rgb = [](float p, float q, float t) -> float {
            if (t < 0) t += 1.0f;
            if (t > 1) t -= 1.0f;
            if (t < 1.0f / 6.0f) return p + (q - p) * 6.0f * t;
            if (t < 0.5f) return q;
            if (t < 2.0f / 3.0f) return p + (q - p) * (2.0f / 3.0f - t) * 6.0f;
            return p;
        };
        float q = l < 0.5f ? l * (1 + s) : l + s - l * s;
        float p = 2 * l - q;
        r = hue2rgb(p, q, h + 1.0f / 3.0f);
        g = hue2rgb(p, q, h);
        b = hue2rgb(p, q, h - 1.0f / 3.0f);
    };

    parallelRows(h, [&](int y) {
        for (int x = 0; x < w; ++x) {
            const uint8_t* s = src.pixelAt(x, y);
            uint8_t* d = dst.pixelAt(x, y);
            float r = s[0] / 255.0f, g = s[1] / 255.0f, b = s[2] / 255.0f;
            float hh, ss, ll;
            rgbToHsl(r, g, b, hh, ss, ll);
            hh = std::fmod(hh + hShift + 1.0f, 1.0f);
            ss = std::clamp(ss * sMul, 0.0f, 1.0f);
            ll = std::clamp(ll + lAdd, 0.0f, 1.0f);
            hslToRgb(hh, ss, ll, r, g, b);
            d[0] = static_cast<uint8_t>(std::clamp(r * 255.0f, 0.0f, 255.0f));
            d[1] = static_cast<uint8_t>(std::clamp(g * 255.0f, 0.0f, 255.0f));
            d[2] = static_cast<uint8_t>(std::clamp(b * 255.0f, 0.0f, 255.0f));
            d[3] = s[3];
        }
    });
}

void GPURenderer::exposureGPU(const PixelBuffer& src, PixelBuffer& dst, double stops) {
    int w = src.width, h = src.height;
    dst.resize(w, h);
    float mul = std::pow(2.0f, static_cast<float>(stops));

    parallelRows(h, [&](int y) {
        for (int x = 0; x < w; ++x) {
            const uint8_t* s = src.pixelAt(x, y);
            uint8_t* d = dst.pixelAt(x, y);
            d[0] = static_cast<uint8_t>(std::clamp(s[0] * mul, 0.0f, 255.0f));
            d[1] = static_cast<uint8_t>(std::clamp(s[1] * mul, 0.0f, 255.0f));
            d[2] = static_cast<uint8_t>(std::clamp(s[2] * mul, 0.0f, 255.0f));
            d[3] = s[3];
        }
    });
}

void GPURenderer::colorBalanceGPU(const PixelBuffer& src, PixelBuffer& dst,
    double shadowsR, double shadowsG, double shadowsB,
    double midR, double midG, double midB,
    double highR, double highG, double highB) {

    int w = src.width, h = src.height;
    dst.resize(w, h);

    parallelRows(h, [&](int y) {
        for (int x = 0; x < w; ++x) {
            const uint8_t* s = src.pixelAt(x, y);
            uint8_t* d = dst.pixelAt(x, y);
            float luma = (s[0] * 0.299f + s[1] * 0.587f + s[2] * 0.114f) / 255.0f;
            float shadowW = std::clamp(1.0f - luma * 2.0f, 0.0f, 1.0f);
            float highW = std::clamp(luma * 2.0f - 1.0f, 0.0f, 1.0f);
            float midW = 1.0f - shadowW - highW;

            float r = s[0] + shadowW * static_cast<float>(shadowsR) * 255.0f
                            + midW * static_cast<float>(midR) * 255.0f
                            + highW * static_cast<float>(highR) * 255.0f;
            float g = s[1] + shadowW * static_cast<float>(shadowsG) * 255.0f
                            + midW * static_cast<float>(midG) * 255.0f
                            + highW * static_cast<float>(highG) * 255.0f;
            float b = s[2] + shadowW * static_cast<float>(shadowsB) * 255.0f
                            + midW * static_cast<float>(midB) * 255.0f
                            + highW * static_cast<float>(highB) * 255.0f;

            d[0] = static_cast<uint8_t>(std::clamp(r, 0.0f, 255.0f));
            d[1] = static_cast<uint8_t>(std::clamp(g, 0.0f, 255.0f));
            d[2] = static_cast<uint8_t>(std::clamp(b, 0.0f, 255.0f));
            d[3] = s[3];
        }
    });
}

// ═══════════════════════════════════════════════════════════════════════
// DISTORT EFFECTS
// ═══════════════════════════════════════════════════════════════════════

void GPURenderer::displacementMapGPU(const PixelBuffer& src, const PixelBuffer& map,
                                      PixelBuffer& dst, double scaleX, double scaleY) {
    int w = src.width, h = src.height;
    dst.resize(w, h);
    float sx = static_cast<float>(scaleX);
    float sy = static_cast<float>(scaleY);

    parallelRows(h, [&](int y) {
        for (int x = 0; x < w; ++x) {
            int mx = std::clamp(x, 0, map.width - 1);
            int my = std::clamp(y, 0, map.height - 1);
            const uint8_t* mp = map.pixelAt(mx, my);
            float dx = (mp[0] / 127.5f - 1.0f) * sx;
            float dy = (mp[1] / 127.5f - 1.0f) * sy;
            int srcX = std::clamp(static_cast<int>(x + dx), 0, w - 1);
            int srcY = std::clamp(static_cast<int>(y + dy), 0, h - 1);
            const uint8_t* s = src.pixelAt(srcX, srcY);
            uint8_t* d = dst.pixelAt(x, y);
            d[0] = s[0]; d[1] = s[1]; d[2] = s[2]; d[3] = s[3];
        }
    });
}

void GPURenderer::waveWarpGPU(const PixelBuffer& src, PixelBuffer& dst,
    int waveType, double amplitude, double wavelength, double speed, double time) {

    int w = src.width, h = src.height;
    dst.resize(w, h);
    float amp = static_cast<float>(amplitude);
    float wl = static_cast<float>(wavelength);
    float spd = static_cast<float>(speed);
    float t = static_cast<float>(time);
    if (wl < 1.0f) wl = 1.0f;

    parallelRows(h, [&](int y) {
        for (int x = 0; x < w; ++x) {
            float phase = (static_cast<float>(y) / wl + t * spd) * 2.0f * static_cast<float>(M_PI);
            float offset = 0.0f;
            switch (waveType) {
                case 0: offset = std::sin(phase) * amp; break;
                case 1: offset = std::fmod(phase, 2.0f * static_cast<float>(M_PI));
                        if (offset > static_cast<float>(M_PI)) offset -= 2.0f * static_cast<float>(M_PI);
                        offset = (offset / static_cast<float>(M_PI)) * amp;
                        break;
                case 2: offset = (std::fmod(phase, 2.0f * static_cast<float>(M_PI)) < static_cast<float>(M_PI) ? 1.0f : -1.0f) * amp; break;
                default: offset = std::sin(phase) * amp; break;
            }
            int srcX = std::clamp(static_cast<int>(x + offset), 0, w - 1);
            const uint8_t* s = src.pixelAt(srcX, y);
            uint8_t* d = dst.pixelAt(x, y);
            d[0] = s[0]; d[1] = s[1]; d[2] = s[2]; d[3] = s[3];
        }
    });
}

void GPURenderer::turbulentDisplaceGPU(const PixelBuffer& src, PixelBuffer& dst,
    double amount, double scale, int complexity, double time) {

    int w = src.width, h = src.height;
    dst.resize(w, h);
    float amt = static_cast<float>(amount);
    float sc = static_cast<float>(scale);
    float t = static_cast<float>(time);

    auto noise2D = [](float x, float y) -> float {
        int ix = static_cast<int>(std::floor(x));
        int iy = static_cast<int>(std::floor(y));
        float fx = x - ix;
        float fy = y - iy;
        auto hash = [](int x, int y) -> float {
            int h = x * 374761393 + y * 668265263;
            h = (h ^ (h >> 13)) * 1274126177;
            return static_cast<float>((h & 0x7fffffff) % 10000) / 10000.0f;
        };
        float a = hash(ix, iy);
        float b = hash(ix + 1, iy);
        float c = hash(ix, iy + 1);
        float d = hash(ix + 1, iy + 1);
        float ux = fx * fx * (3.0f - 2.0f * fx);
        float uy = fy * fy * (3.0f - 2.0f * fy);
        return a * (1 - ux) * (1 - uy) + b * ux * (1 - uy) + c * (1 - ux) * uy + d * ux * uy;
    };

    parallelRows(h, [&](int y) {
        for (int x = 0; x < w; ++x) {
            float nx = static_cast<float>(x) / sc;
            float ny = static_cast<float>(y) / sc + t;
            float disp = 0.0f;
            float amp2 = 1.0f;
            float freq = 1.0f;
            for (int o = 0; o < complexity; ++o) {
                disp += noise2D(nx * freq, ny * freq) * amp2;
                amp2 *= 0.5f;
                freq *= 2.0f;
            }
            disp = (disp - 0.5f) * amt;
            int srcX = std::clamp(static_cast<int>(x + disp), 0, w - 1);
            int srcY = std::clamp(static_cast<int>(y + disp * 0.5f), 0, h - 1);
            const uint8_t* s = src.pixelAt(srcX, srcY);
            uint8_t* d = dst.pixelAt(x, y);
            d[0] = s[0]; d[1] = s[1]; d[2] = s[2]; d[3] = s[3];
        }
    });
}

void GPURenderer::bulgeGPU(const PixelBuffer& src, PixelBuffer& dst,
    double centerX, double centerY, double radius, double amount) {

    int w = src.width, h = src.height;
    dst.resize(w, h);
    float cx = static_cast<float>(centerX);
    float cy = static_cast<float>(centerY);
    float rad = static_cast<float>(radius);
    float amt = static_cast<float>(amount);

    parallelRows(h, [&](int y) {
        for (int x = 0; x < w; ++x) {
            float dx = static_cast<float>(x) - cx;
            float dy = static_cast<float>(y) - cy;
            float dist = std::sqrt(dx * dx + dy * dy);
            float srcX = static_cast<float>(x);
            float srcY = static_cast<float>(y);
            if (dist < rad && rad > 0) {
                float pct = dist / rad;
                float power = 1.0f + amt * (1.0f - pct * pct);
                srcX = cx + dx / power;
                srcY = cy + dy / power;
            }
            int sx = std::clamp(static_cast<int>(srcX), 0, w - 1);
            int sy = std::clamp(static_cast<int>(srcY), 0, h - 1);
            const uint8_t* s = src.pixelAt(sx, sy);
            uint8_t* d = dst.pixelAt(x, y);
            d[0] = s[0]; d[1] = s[1]; d[2] = s[2]; d[3] = s[3];
        }
    });
}

void GPURenderer::twirlGPU(const PixelBuffer& src, PixelBuffer& dst,
    double centerX, double centerY, double angle, double radius) {

    int w = src.width, h = src.height;
    dst.resize(w, h);
    float cx = static_cast<float>(centerX);
    float cy = static_cast<float>(centerY);
    float ang = static_cast<float>(angle) * static_cast<float>(M_PI) / 180.0f;
    float rad = static_cast<float>(radius);

    parallelRows(h, [&](int y) {
        for (int x = 0; x < w; ++x) {
            float dx = static_cast<float>(x) - cx;
            float dy = static_cast<float>(y) - cy;
            float dist = std::sqrt(dx * dx + dy * dy);
            float srcX = static_cast<float>(x);
            float srcY = static_cast<float>(y);
            if (dist < rad && rad > 0) {
                float pct = 1.0f - dist / rad;
                float twist = ang * pct * pct;
                float cosT = std::cos(twist);
                float sinT = std::sin(twist);
                srcX = cx + dx * cosT - dy * sinT;
                srcY = cy + dx * sinT + dy * cosT;
            }
            int sx = std::clamp(static_cast<int>(srcX), 0, w - 1);
            int sy = std::clamp(static_cast<int>(srcY), 0, h - 1);
            const uint8_t* s = src.pixelAt(sx, sy);
            uint8_t* d = dst.pixelAt(x, y);
            d[0] = s[0]; d[1] = s[1]; d[2] = s[2]; d[3] = s[3];
        }
    });
}

// ═══════════════════════════════════════════════════════════════════════
// GENERATE EFFECTS
// ═══════════════════════════════════════════════════════════════════════

void GPURenderer::fractalNoiseGPU(PixelBuffer& dst, double scale, int octaves,
                                   double lacunarity, double gain, double time) {
    int w = dst.width, h = dst.height;

    auto noise2D = [](float x, float y) -> float {
        int ix = static_cast<int>(std::floor(x));
        int iy = static_cast<int>(std::floor(y));
        float fx = x - ix;
        float fy = y - iy;
        auto hash = [](int x, int y) -> float {
            int h = x * 374761393 + y * 668265263;
            h = (h ^ (h >> 13)) * 1274126177;
            return static_cast<float>((h & 0x7fffffff) % 10000) / 10000.0f;
        };
        float a = hash(ix, iy);
        float b = hash(ix + 1, iy);
        float c = hash(ix, iy + 1);
        float d = hash(ix + 1, iy + 1);
        float ux = fx * fx * (3.0f - 2.0f * fx);
        float uy = fy * fy * (3.0f - 2.0f * fy);
        return a * (1 - ux) * (1 - uy) + b * ux * (1 - uy) + c * (1 - ux) * uy + d * ux * uy;
    };

    float sc = static_cast<float>(scale);
    float lac = static_cast<float>(lacunarity);
    float g = static_cast<float>(gain);
    float t = static_cast<float>(time);

    parallelRows(h, [&](int y) {
        for (int x = 0; x < w; ++x) {
            float nx = static_cast<float>(x) / sc;
            float ny = static_cast<float>(y) / sc + t;
            float val = 0.0f;
            float amp = 1.0f;
            float freq = 1.0f;
            float maxAmp = 0.0f;
            for (int o = 0; o < octaves; ++o) {
                val += noise2D(nx * freq, ny * freq) * amp;
                maxAmp += amp;
                amp *= g;
                freq *= lac;
            }
            val = val / maxAmp;
            val = std::clamp(val, 0.0f, 1.0f);
            uint8_t v = static_cast<uint8_t>(val * 255.0f);
            uint8_t* d = dst.pixelAt(x, y);
            d[0] = v; d[1] = v; d[2] = v; d[3] = 255;
        }
    });
}

void GPURenderer::cloudGPU(PixelBuffer& dst, double scale, double time) {
    fractalNoiseGPU(dst, scale, 6, 2.0, 0.5, time);
    int w = dst.width, h = dst.height;
    parallelRows(h, [&](int y) {
        for (int x = 0; x < w; ++x) {
            uint8_t* d = dst.pixelAt(x, y);
            float v = d[0] / 255.0f;
            d[0] = static_cast<uint8_t>(std::clamp(v * 180.0f + 75.0f, 0.0f, 255.0f));
            d[1] = static_cast<uint8_t>(std::clamp(v * 180.0f + 75.0f, 0.0f, 255.0f));
            d[2] = static_cast<uint8_t>(std::clamp(v * 200.0f + 55.0f, 0.0f, 255.0f));
        }
    });
}

void GPURenderer::checkerboardGPU(PixelBuffer& dst, int size, const Color& color1, const Color& color2) {
    int w = dst.width, h = dst.height;
    if (size <= 0) size = 1;
    uint8_t r1 = static_cast<uint8_t>(std::clamp(color1.r, 0.0, 1.0) * 255.0);
    uint8_t g1 = static_cast<uint8_t>(std::clamp(color1.g, 0.0, 1.0) * 255.0);
    uint8_t b1 = static_cast<uint8_t>(std::clamp(color1.b, 0.0, 1.0) * 255.0);
    uint8_t r2 = static_cast<uint8_t>(std::clamp(color2.r, 0.0, 1.0) * 255.0);
    uint8_t g2 = static_cast<uint8_t>(std::clamp(color2.g, 0.0, 1.0) * 255.0);
    uint8_t b2 = static_cast<uint8_t>(std::clamp(color2.b, 0.0, 1.0) * 255.0);

    parallelRows(h, [&](int y) {
        for (int x = 0; x < w; ++x) {
            bool even = ((x / size) + (y / size)) % 2 == 0;
            uint8_t* d = dst.pixelAt(x, y);
            d[0] = even ? r1 : r2;
            d[1] = even ? g1 : g2;
            d[2] = even ? b1 : b2;
            d[3] = 255;
        }
    });
}

void GPURenderer::lensFlareGPU(PixelBuffer& dst, double centerX, double centerY,
                                double intensity, double size) {
    int w = dst.width, h = dst.height;
    float cx = static_cast<float>(centerX) * w;
    float cy = static_cast<float>(centerY) * h;
    float sz = static_cast<float>(size) * 100.0f;
    float inten = static_cast<float>(intensity);

    parallelRows(h, [&](int y) {
        for (int x = 0; x < w; ++x) {
            float dx = static_cast<float>(x) - cx;
            float dy = static_cast<float>(y) - cy;
            float dist = std::sqrt(dx * dx + dy * dy);
            float glow = std::exp(-dist * dist / (sz * sz)) * inten;
            glow = std::clamp(glow, 0.0f, 1.0f);
            uint8_t* d = dst.pixelAt(x, y);
            d[0] = static_cast<uint8_t>(std::clamp(d[0] + glow * 255.0f, 0.0f, 255.0f));
            d[1] = static_cast<uint8_t>(std::clamp(d[1] + glow * 240.0f, 0.0f, 255.0f));
            d[2] = static_cast<uint8_t>(std::clamp(d[2] + glow * 200.0f, 0.0f, 255.0f));
        }
    });
}

// ═══════════════════════════════════════════════════════════════════════
// STYLIZE EFFECTS
// ═══════════════════════════════════════════════════════════════════════

void GPURenderer::glowGPU(const PixelBuffer& src, PixelBuffer& dst,
    double threshold, double radius, double intensity) {

    int w = src.width, h = src.height;
    PixelBuffer bright;
    bright.resize(w, h);
    float thr = static_cast<float>(threshold) * 255.0f;

    parallelRows(h, [&](int y) {
        for (int x = 0; x < w; ++x) {
            const uint8_t* s = src.pixelAt(x, y);
            uint8_t* d = bright.pixelAt(x, y);
            float luma = s[0] * 0.299f + s[1] * 0.587f + s[2] * 0.114f;
            if (luma > thr) {
                float mul = (luma - thr) / (255.0f - thr);
                d[0] = static_cast<uint8_t>(std::clamp(s[0] * mul, 0.0f, 255.0f));
                d[1] = static_cast<uint8_t>(std::clamp(s[1] * mul, 0.0f, 255.0f));
                d[2] = static_cast<uint8_t>(std::clamp(s[2] * mul, 0.0f, 255.0f));
                d[3] = s[3];
            } else {
                d[0] = d[1] = d[2] = 0;
                d[3] = 0;
            }
        }
    });

    PixelBuffer blurred;
    gaussianBlurGPU(bright, blurred, std::max(1, static_cast<int>(radius)));

    float inten2 = static_cast<float>(intensity);
    dst.resize(w, h);
    parallelRows(h, [&](int y) {
        for (int x = 0; x < w; ++x) {
            const uint8_t* s = src.pixelAt(x, y);
            const uint8_t* g = blurred.pixelAt(x, y);
            uint8_t* d = dst.pixelAt(x, y);
            d[0] = static_cast<uint8_t>(std::clamp(s[0] + g[0] * inten2, 0.0f, 255.0f));
            d[1] = static_cast<uint8_t>(std::clamp(s[1] + g[1] * inten2, 0.0f, 255.0f));
            d[2] = static_cast<uint8_t>(std::clamp(s[2] + g[2] * inten2, 0.0f, 255.0f));
            d[3] = s[3];
        }
    });
}

void GPURenderer::dropShadowGPU(const PixelBuffer& src, PixelBuffer& dst,
    double angle, double distance, double softness, const Color& color) {

    int w = src.width, h = src.height;
    PixelBuffer shadow;
    shadow.resize(w, h);
    std::memset(shadow.data.data(), 0, shadow.data.size());

    double rad = angle * M_PI / 180.0;
    int offX = static_cast<int>(std::cos(rad) * distance);
    int offY = static_cast<int>(std::sin(rad) * distance);
    uint8_t sr = static_cast<uint8_t>(std::clamp(color.r, 0.0, 1.0) * 255.0);
    uint8_t sg = static_cast<uint8_t>(std::clamp(color.g, 0.0, 1.0) * 255.0);
    uint8_t sb = static_cast<uint8_t>(std::clamp(color.b, 0.0, 1.0) * 255.0);

    parallelRows(h, [&](int y) {
        for (int x = 0; x < w; ++x) {
            const uint8_t* s = src.pixelAt(x, y);
            if (s[3] > 128) {
                int dx2 = std::clamp(x + offX, 0, w - 1);
                int dy2 = std::clamp(y + offY, 0, h - 1);
                uint8_t* d = shadow.pixelAt(dx2, dy2);
                d[0] = sr; d[1] = sg; d[2] = sb; d[3] = 200;
            }
        }
    });

    if (softness > 0) {
        PixelBuffer blurred;
        gaussianBlurGPU(shadow, blurred, std::max(1, static_cast<int>(softness)));
        shadow = std::move(blurred);
    }

    dst.resize(w, h);
    parallelRows(h, [&](int y) {
        for (int x = 0; x < w; ++x) {
            const uint8_t* bg = shadow.pixelAt(x, y);
            const uint8_t* fg = src.pixelAt(x, y);
            uint8_t* d = dst.pixelAt(x, y);
            float alpha = fg[3] / 255.0f;
            d[0] = static_cast<uint8_t>(std::clamp(fg[0] * alpha + bg[0] * (1.0f - alpha), 0.0f, 255.0f));
            d[1] = static_cast<uint8_t>(std::clamp(fg[1] * alpha + bg[1] * (1.0f - alpha), 0.0f, 255.0f));
            d[2] = static_cast<uint8_t>(std::clamp(fg[2] * alpha + bg[2] * (1.0f - alpha), 0.0f, 255.0f));
            d[3] = static_cast<uint8_t>(std::clamp((alpha + bg[3] / 255.0f * (1.0f - alpha)) * 255.0f, 0.0f, 255.0f));
        }
    });
}

void GPURenderer::embossGPU(const PixelBuffer& src, PixelBuffer& dst, double angle, double depth) {
    int w = src.width, h = src.height;
    dst.resize(w, h);
    double rad = angle * M_PI / 180.0;
    int dx1 = static_cast<int>(std::cos(rad));
    int dy1 = static_cast<int>(std::sin(rad));
    float dep = static_cast<float>(depth) * 0.5f;

    parallelRows(h, [&](int y) {
        for (int x = 0; x < w; ++x) {
            int sx1 = std::clamp(x + dx1, 0, w - 1);
            int sy1 = std::clamp(y + dy1, 0, h - 1);
            int sx2 = std::clamp(x - dx1, 0, w - 1);
            int sy2 = std::clamp(y - dy1, 0, h - 1);
            const uint8_t* p1 = src.pixelAt(sx1, sy1);
            const uint8_t* p2 = src.pixelAt(sx2, sy2);
            float l1 = p1[0] * 0.299f + p1[1] * 0.587f + p1[2] * 0.114f;
            float l2 = p2[0] * 0.299f + p2[1] * 0.587f + p2[2] * 0.114f;
            float v = std::clamp((l1 - l2) * dep + 128.0f, 0.0f, 255.0f);
            uint8_t* d = dst.pixelAt(x, y);
            d[0] = d[1] = d[2] = static_cast<uint8_t>(v);
            d[3] = src.pixelAt(x, y)[3];
        }
    });
}

void GPURenderer::findEdgesGPU(const PixelBuffer& src, PixelBuffer& dst) {
    int w = src.width, h = src.height;
    dst.resize(w, h);

    parallelRows(h, [&](int y) {
        for (int x = 0; x < w; ++x) {
            float gx = 0, gy = 0;
            for (int dy = -1; dy <= 1; ++dy) {
                for (int dx = -1; dx <= 1; ++dx) {
                    int sx = std::clamp(x + dx, 0, w - 1);
                    int sy = std::clamp(y + dy, 0, h - 1);
                    const uint8_t* p = src.pixelAt(sx, sy);
                    float luma = p[0] * 0.299f + p[1] * 0.587f + p[2] * 0.114f;
                    static const float sobelX[3][3] = {{-1,0,1},{-2,0,2},{-1,0,1}};
                    static const float sobelY[3][3] = {{-1,-2,-1},{0,0,0},{1,2,1}};
                    gx += luma * sobelX[dy + 1][dx + 1];
                    gy += luma * sobelY[dy + 1][dx + 1];
                }
            }
            float mag = std::clamp(std::sqrt(gx * gx + gy * gy), 0.0f, 255.0f);
            uint8_t* d = dst.pixelAt(x, y);
            d[0] = d[1] = d[2] = static_cast<uint8_t>(255.0f - mag);
            d[3] = 255;
        }
    });
}

void GPURenderer::posterizeGPU(const PixelBuffer& src, PixelBuffer& dst, int levels) {
    int w = src.width, h = src.height;
    dst.resize(w, h);
    int lev = std::max(2, levels);
    float scale = 255.0f / static_cast<float>(lev - 1);

    parallelRows(h, [&](int y) {
        for (int x = 0; x < w; ++x) {
            const uint8_t* s = src.pixelAt(x, y);
            uint8_t* d = dst.pixelAt(x, y);
            d[0] = static_cast<uint8_t>(std::clamp(std::round(s[0] / scale) * scale, 0.0f, 255.0f));
            d[1] = static_cast<uint8_t>(std::clamp(std::round(s[1] / scale) * scale, 0.0f, 255.0f));
            d[2] = static_cast<uint8_t>(std::clamp(std::round(s[2] / scale) * scale, 0.0f, 255.0f));
            d[3] = s[3];
        }
    });
}

void GPURenderer::thresholdGPU(const PixelBuffer& src, PixelBuffer& dst, double threshold) {
    int w = src.width, h = src.height;
    dst.resize(w, h);
    float thr = static_cast<float>(threshold) * 255.0f;

    parallelRows(h, [&](int y) {
        for (int x = 0; x < w; ++x) {
            const uint8_t* s = src.pixelAt(x, y);
            float luma = s[0] * 0.299f + s[1] * 0.587f + s[2] * 0.114f;
            uint8_t v = luma >= thr ? 255 : 0;
            uint8_t* d = dst.pixelAt(x, y);
            d[0] = d[1] = d[2] = v;
            d[3] = s[3];
        }
    });
}

void GPURenderer::noiseGPU(PixelBuffer& dst, double amount, bool monochromatic, double time) {
    int w = dst.width, h = dst.height;
    float amt = static_cast<float>(amount) * 255.0f;
    float t = static_cast<float>(time);

    parallelRows(h, [&](int y) {
        std::mt19937 rng(static_cast<unsigned>(y * 1000 + static_cast<int>(t * 100)));
        std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
        for (int x = 0; x < w; ++x) {
            float n = dist(rng) * amt;
            uint8_t* d = dst.pixelAt(x, y);
            if (monochromatic) {
                uint8_t v = static_cast<uint8_t>(std::clamp(n, -128.0f, 127.0f) + 128.0f);
                d[0] = d[1] = d[2] = v;
            } else {
                float r = dist(rng) * amt;
                float g = dist(rng) * amt;
                float b = dist(rng) * amt;
                d[0] = static_cast<uint8_t>(std::clamp(r, -128.0f, 127.0f) + 128.0f);
                d[1] = static_cast<uint8_t>(std::clamp(g, -128.0f, 127.0f) + 128.0f);
                d[2] = static_cast<uint8_t>(std::clamp(b, -128.0f, 127.0f) + 128.0f);
            }
            d[3] = 255;
        }
    });
}

void GPURenderer::mosaicGPU(const PixelBuffer& src, PixelBuffer& dst, int blockWidth, int blockHeight) {
    int w = src.width, h = src.height;
    dst.resize(w, h);
    if (blockWidth <= 0) blockWidth = 1;
    if (blockHeight <= 0) blockHeight = 1;

    parallelRows(h, [&](int y) {
        for (int x = 0; x < w; ++x) {
            int bx = (x / blockWidth) * blockWidth;
            int by = (y / blockHeight) * blockHeight;
            bx = std::clamp(bx, 0, w - 1);
            by = std::clamp(by, 0, h - 1);
            const uint8_t* s = src.pixelAt(bx, by);
            uint8_t* d = dst.pixelAt(x, y);
            d[0] = s[0]; d[1] = s[1]; d[2] = s[2]; d[3] = s[3];
        }
    });
}

// ═══════════════════════════════════════════════════════════════════════
// KEYING
// ═══════════════════════════════════════════════════════════════════════

void GPURenderer::keylightGPU(const PixelBuffer& src, PixelBuffer& dst,
    const Color& keyColor, double tolerance, double softness, double spillSuppression) {

    int w = src.width, h = src.height;
    dst.resize(w, h);
    float kr = static_cast<float>(keyColor.r) * 255.0f;
    float kg = static_cast<float>(keyColor.g) * 255.0f;
    float kb = static_cast<float>(keyColor.b) * 255.0f;
    float tol = static_cast<float>(tolerance) * 255.0f;
    float soft = static_cast<float>(softness) * 255.0f;
    float spill = static_cast<float>(spillSuppression);

    parallelRows(h, [&](int y) {
        for (int x = 0; x < w; ++x) {
            const uint8_t* s = src.pixelAt(x, y);
            float dr = s[0] - kr;
            float dg = s[1] - kg;
            float db = s[2] - kb;
            float dist = std::sqrt(dr * dr + dg * dg + db * db);
            float alpha = 1.0f;
            if (dist < tol) {
                alpha = 0.0f;
            } else if (dist < tol + soft) {
                alpha = (dist - tol) / soft;
            }
            alpha = std::clamp(alpha, 0.0f, 1.0f);
            float maxC = std::max({static_cast<float>(s[0]), static_cast<float>(s[1]), static_cast<float>(s[2])});
            float spillMul = 1.0f - spill * std::max(0.0f, 1.0f - alpha);
            uint8_t* d = dst.pixelAt(x, y);
            d[0] = static_cast<uint8_t>(std::clamp(s[0] * spillMul, 0.0f, 255.0f));
            d[1] = static_cast<uint8_t>(std::clamp(s[1] * spillMul, 0.0f, 255.0f));
            d[2] = static_cast<uint8_t>(std::clamp(s[2] * spillMul, 0.0f, 255.0f));
            d[3] = static_cast<uint8_t>(alpha * 255.0f);
            (void)maxC;
        }
    });
}

// ═══════════════════════════════════════════════════════════════════════
// BLUR HELPERS
// ═══════════════════════════════════════════════════════════════════════

void GPURenderer::radialBlurGPU(const PixelBuffer& src, PixelBuffer& dst,
    double centerX, double centerY, int amount, int type) {
    int w = src.width, h = src.height;
    dst.resize(w, h);
    float cx = static_cast<float>(centerX) * w;
    float cy = static_cast<float>(centerY) * h;
    float amt = static_cast<float>(amount) / 100.0f;

    parallelRows(h, [&](int y) {
        for (int x = 0; x < w; ++x) {
            float dx = static_cast<float>(x) - cx;
            float dy = static_cast<float>(y) - cy;
            float dist = std::sqrt(dx * dx + dy * dy);
            float angle = std::atan2(dy, dx);
            float r = 0, g = 0, b = 0, a = 0;
            int count = 0;
            int samples = std::max(1, amount / 2);
            for (int s = 0; s < samples; ++s) {
                float t = static_cast<float>(s) / static_cast<float>(samples) - 0.5f;
                float sa = angle + t * amt;
                float sx2 = cx + std::cos(sa) * dist;
                float sy2 = cy + std::sin(sa) * dist;
                int pixX = std::clamp(static_cast<int>(sx2), 0, w - 1);
                int pixY = std::clamp(static_cast<int>(sy2), 0, h - 1);
                const uint8_t* p = src.pixelAt(pixX, pixY);
                r += p[0]; g += p[1]; b += p[2]; a += p[3];
                count++;
            }
            float inv = 1.0f / static_cast<float>(count);
            uint8_t* d = dst.pixelAt(x, y);
            d[0] = static_cast<uint8_t>(std::clamp(r * inv, 0.0f, 255.0f));
            d[1] = static_cast<uint8_t>(std::clamp(g * inv, 0.0f, 255.0f));
            d[2] = static_cast<uint8_t>(std::clamp(b * inv, 0.0f, 255.0f));
            d[3] = static_cast<uint8_t>(std::clamp(a * inv, 0.0f, 255.0f));
            (void)type;
        }
    });
}

void GPURenderer::ccRadialBlurGPU(const PixelBuffer& src, PixelBuffer& dst,
    double centerX, double centerY, int amount) {
    radialBlurGPU(src, dst, centerX, centerY, amount, 0);
}

void GPURenderer::vectorBlurGPU(const PixelBuffer& src, PixelBuffer& dst,
    double amount, double angle) {
    directionalBlurGPU(src, dst, angle, std::max(1, static_cast<int>(amount)));
}

// ═══════════════════════════════════════════════════════════════════════
// TRANSITION EFFECTS
// ═══════════════════════════════════════════════════════════════════════

void GPURenderer::linearWipeGPU(const PixelBuffer& src, PixelBuffer& dst,
    double angle, double completion, double feather) {

    int w = src.width, h = src.height;
    dst.resize(w, h);
    double rad = angle * M_PI / 180.0;
    double nx = std::cos(rad);
    double ny = std::sin(rad);
    double comp = std::clamp(completion / 100.0, 0.0, 1.0);
    double feat = feather;
    double maxDist = std::abs(nx) * w + std::abs(ny) * h;

    parallelRows(h, [&](int y) {
        for (int x = 0; x < w; ++x) {
            double px = static_cast<double>(x) / w - 0.5;
            double py = static_cast<double>(y) / h - 0.5;
            double dot = px * nx + py * ny;
            double edge = (comp - 0.5) * 2.0;
            double dist = dot - edge;
            double alpha = 1.0;
            if (feat > 0) {
                alpha = std::clamp((dist + feat * 0.5) / feat, 0.0, 1.0);
            } else {
                alpha = dist > 0 ? 1.0 : 0.0;
            }
            uint8_t* d = dst.pixelAt(x, y);
            const uint8_t* s = src.pixelAt(x, y);
            d[0] = s[0]; d[1] = s[1]; d[2] = s[2];
            d[3] = static_cast<uint8_t>(alpha * s[3]);
            (void)maxDist;
        }
    });
}

void GPURenderer::radialWipeGPU(const PixelBuffer& src, PixelBuffer& dst,
    double completion, double center, double startAngle, double feather) {

    int w = src.width, h = src.height;
    dst.resize(w, h);
    float cx = w * 0.5f;
    float cy = h * 0.5f;
    float comp = std::clamp(static_cast<float>(completion) / 100.0f, 0.0f, 1.0f);
    float start = static_cast<float>(startAngle) * static_cast<float>(M_PI) / 180.0f;
    float feat = static_cast<float>(feather) * static_cast<float>(M_PI) / 180.0f;
    (void)center;

    parallelRows(h, [&](int y) {
        for (int x = 0; x < w; ++x) {
            float dx = static_cast<float>(x) - cx;
            float dy = static_cast<float>(y) - cy;
            float angle = std::atan2(dy, dx) - start;
            while (angle < 0) angle += 2.0f * static_cast<float>(M_PI);
            float sweep = comp * 2.0f * static_cast<float>(M_PI);
            float diff = angle - sweep;
            float alpha = 1.0f;
            if (diff > 0) {
                if (feat > 0) {
                    alpha = std::clamp(1.0f - diff / feat, 0.0f, 1.0f);
                } else {
                    alpha = 0.0f;
                }
            }
            uint8_t* d = dst.pixelAt(x, y);
            const uint8_t* s = src.pixelAt(x, y);
            d[0] = s[0]; d[1] = s[1]; d[2] = s[2];
            d[3] = static_cast<uint8_t>(alpha * s[3]);
        }
    });
}

} // namespace FreeEffect
