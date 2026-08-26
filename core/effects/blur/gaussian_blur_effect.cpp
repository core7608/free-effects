#include <algorithm>
#include "../effect_registry.h"
#include "gaussian_blur_effect.h"
#include <cmath>
#include <vector>

namespace FreeEffect {

static EffectRegistrar<GaussianBlurEffect> s_reg("Gaussian Blur", "Blur & Sharpen");

GaussianBlurEffect::GaussianBlurEffect() {
    addParameter(EffectParameter::makeFloat("radius", "Blurriness", 0.0, 200.0, 5.0));
}

std::vector<ParameterGroup> GaussianBlurEffect::getParameterGroups() const {
    return {{getName(), {EffectParameter::makeFloat("radius", "Blurriness", 0.0, 200.0, false)}}};
}

std::unique_ptr<Effect> GaussianBlurEffect::clone() const {
    auto e = std::make_unique<GaussianBlurEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void GaussianBlurEffect::blurLine(uint8_t* line, int len, const float* kernel, int ksize) {
    int half = ksize / 2;
    std::vector<float> tmp(len * 4);
    for (int x = 0; x < len; x++) {
        float r = 0, g = 0, b = 0, a = 0;
        for (int k = 0; k < ksize; k++) {
            int sx = std::clamp(x + k - half, 0, len - 1);
            float w = kernel[k];
            r += line[sx * 4 + 0] * w;
            g += line[sx * 4 + 1] * w;
            b += line[sx * 4 + 2] * w;
            a += line[sx * 4 + 3] * w;
        }
        tmp[x * 4 + 0] = r;
        tmp[x * 4 + 1] = g;
        tmp[x * 4 + 2] = b;
        tmp[x * 4 + 3] = a;
    }
    for (int x = 0; x < len * 4; x++) {
        line[x] = static_cast<uint8_t>(std::clamp(static_cast<double>(tmp[x]), 0.0, 255.0));
    }
}

void GaussianBlurEffect::gaussianBlur(PixelBuffer& buf, int radius) {
    if (radius <= 0) return;
    int ksize = radius * 2 + 1;
    std::vector<float> kernel(ksize);
    float sigma = radius / 3.0f;
    float sum = 0;
    for (int i = 0; i < ksize; i++) {
        float x = static_cast<float>(i - radius);
        kernel[i] = std::exp(-(x * x) / (2.0f * sigma * sigma));
        sum += kernel[i];
    }
    for (auto& k : kernel) k /= sum;

    PixelBuffer tmp;
    tmp.resize(buf.width, buf.height);

    for (int y = 0; y < buf.height; y++) {
        const uint8_t* src = buf.pixelAt(0, y);
        uint8_t* dst = tmp.pixelAt(0, y);
        std::memcpy(dst, src, buf.width * 4);
        blurLine(dst, buf.width, kernel.data(), ksize);
    }

    for (int x = 0; x < buf.width; x++) {
        std::vector<float> col(buf.height * 4);
        for (int y = 0; y < buf.height; y++) {
            const uint8_t* p = tmp.pixelAt(x, y);
            col[y * 4 + 0] = p[0];
            col[y * 4 + 1] = p[1];
            col[y * 4 + 2] = p[2];
            col[y * 4 + 3] = p[3];
        }
        for (int y = 0; y < buf.height; y++) {
            float r = 0, g = 0, b = 0, a = 0;
            for (int k = 0; k < ksize; k++) {
                int sy = std::clamp(y + k - radius, 0, buf.height - 1);
                float w = kernel[k];
                r += col[sy * 4 + 0] * w;
                g += col[sy * 4 + 1] * w;
                b += col[sy * 4 + 2] * w;
                a += col[sy * 4 + 3] * w;
            }
            uint8_t* p = buf.pixelAt(x, y);
            p[0] = static_cast<uint8_t>(std::clamp(static_cast<double>(r), 0.0, 255.0));
            p[1] = static_cast<uint8_t>(std::clamp(static_cast<double>(g), 0.0, 255.0));
            p[2] = static_cast<uint8_t>(std::clamp(static_cast<double>(b), 0.0, 255.0));
            p[3] = static_cast<uint8_t>(std::clamp(static_cast<double>(a), 0.0, 255.0));
        }
    }
}

void GaussianBlurEffect::render(PixelBuffer& buffer, double time) {
    int radius = static_cast<int>(getFloatParam("radius"));
    gaussianBlur(buffer, radius);
}

} // namespace FreeEffect
