#include "../effect_registry.h"
#include "camera_lens_blur_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<CameraLensBlurEffect> s_reg("Camera Lens Blur", "Blur & Sharpen");

CameraLensBlurEffect::CameraLensBlurEffect() {
    addParameter(EffectParameter::makeInt("blurRadius", "Blur Radius", 1, 100, 5));
    addParameter(EffectParameter::makeInt("irisSides", "Iris Shape", 3, 10, 6));
    addParameter(EffectParameter::makeFloat("irisRotation", "Iris Rotation", 0.0, 360.0, 0.0));
    addParameter(EffectParameter::makeFloat("irisRoundness", "Iris Roundness", 0.0, 100.0, 50.0));
    addParameter(EffectParameter::makeFloat("specularThreshold", "Specular Threshold", 0.0, 255.0, 200.0));
    addParameter(EffectParameter::makeFloat("specularGain", "Specular Gain", 0.0, 10.0, 1.0));
    addParameter(EffectParameter::makeFloat("boostLight", "Boost Light", 0.0, 1.0, 0.0));
}

std::unique_ptr<Effect> CameraLensBlurEffect::clone() const {
    auto e = std::make_unique<CameraLensBlurEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void CameraLensBlurEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    int radius = getIntParam("blurRadius");
    if (radius <= 0) return;

    int sides = std::clamp(getIntParam("irisSides"), 3, 10);
    float rot = getFloatParam("irisRotation") * 3.14159265f / 180.0f;
    float roundness = getFloatParam("irisRoundness") / 100.0f;
    float specThresh = getFloatParam("specularThreshold");
    float specGain = getFloatParam("specularGain");

    int ksize = radius * 2 + 1;
    std::vector<float> kernel(ksize * ksize);
    float sum = 0.0f;
    for (int ky = -radius; ky <= radius; ky++) {
        for (int kx = -radius; kx <= radius; kx++) {
            float nx = static_cast<float>(kx) / radius;
            float ny = static_cast<float>(ky) / radius;
            float angle = std::atan2(ny, nx) + rot;
            float dist = std::sqrt(nx * nx + ny * ny);
            float a = sides > 0 ? static_cast<float>(sides) : 6.0f;
            float polyR = std::cos(3.14159265f / a) /
                          std::cos(std::fmod(angle + 3.14159265f / a, 2.0f * 3.14159265f / a) - 3.14159265f / a);
            float shape = dist / std::max(polyR, 0.01f);
            shape = shape * (1.0f - roundness) + dist * roundness;
            float w = (shape <= 1.0f) ? 1.0f - shape : 0.0f;
            kernel[(ky + radius) * ksize + (kx + radius)] = w;
            sum += w;
        }
    }
    if (sum > 0.0f) {
        for (auto& k : kernel) k /= sum;
    }

    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);
    std::copy(buffer.data.begin(), buffer.data.end(), tmp.data.begin());

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            float r = 0, g = 0, b = 0, a = 0;
            for (int ky = -radius; ky <= radius; ky++) {
                for (int kx = -radius; kx <= radius; kx++) {
                    int sx = std::clamp(x + kx, 0, buffer.width - 1);
                    int sy = std::clamp(y + ky, 0, buffer.height - 1);
                    float w = kernel[(ky + radius) * ksize + (kx + radius)];
                    const uint8_t* p = tmp.pixelAt(sx, sy);
                    r += p[0] * w;
                    g += p[1] * w;
                    b += p[2] * w;
                    a += p[3] * w;
                }
            }

            uint8_t* dst = buffer.pixelAt(x, y);
            const uint8_t* src = tmp.pixelAt(x, y);
            float origBright = (src[0] + src[1] + src[2]) / 3.0f;
            float specMult = (origBright > specThresh) ? specGain : 1.0f;
            dst[0] = static_cast<uint8_t>(std::clamp(r * specMult, 0.0f, 255.0f));
            dst[1] = static_cast<uint8_t>(std::clamp(g * specMult, 0.0f, 255.0f));
            dst[2] = static_cast<uint8_t>(std::clamp(b * specMult, 0.0f, 255.0f));
            dst[3] = static_cast<uint8_t>(std::clamp(a, 0.0f, 255.0f));
        }
    }
}

} // namespace FreeEffect
