#include "../effect_registry.h"
#include "fractal_noise_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<FractalNoiseEffect> s_reg("Fractal Noise", "Stylize");

FractalNoiseEffect::FractalNoiseEffect() {
    addParameter(EffectParameter::makeDropdown("type", "Noise Type", {"Basic", "Turbulent", "Dynamic"}, 0));
    addParameter(EffectParameter::makeFloat("contrast", "Contrast", 0.0, 500.0, 100.0));
    addParameter(EffectParameter::makeFloat("brightness", "Brightness", -200.0, 200.0, 0.0));
    addParameter(EffectParameter::makeInt("complexity", "Complexity", 1, 30, 6));
    addParameter(EffectParameter::makeInt("seed", "Random Seed", 0, 9999, 0));
    addParameter(EffectParameter::makeFloat("offset", "Offset Turbulence", -1000.0, 1000.0, 0.0));
    addParameter(EffectParameter::makeFloat("scale", "Scale", 1.0, 500.0, 100.0));
}

std::vector<ParameterGroup> FractalNoiseEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeDropdown("type", "Noise Type", {"Basic", "Turbulent", "Dynamic"}, 0),
        EffectParameter::makeFloat("contrast", "Contrast", 0.0, 500.0, false),
        EffectParameter::makeFloat("brightness", "Brightness", -200.0, 200.0, false),
        EffectParameter::makeInt("complexity", "Complexity", 1, 30, false),
        EffectParameter::makeInt("seed", "Random Seed", 0, 9999, false),
        EffectParameter::makeFloat("offset", "Offset Turbulence", -1000.0, 1000.0, false),
        EffectParameter::makeFloat("scale", "Scale", 1.0, 500.0, false)
    }}};
}

std::unique_ptr<Effect> FractalNoiseEffect::clone() const {
    auto e = std::make_unique<FractalNoiseEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

float FractalNoiseEffect::noise2D(float x, float y) const {
    int ix = static_cast<int>(std::floor(x));
    int iy = static_cast<int>(std::floor(y));
    float fx = x - ix;
    float fy = y - iy;
    fx = fx * fx * (3.0f - 2.0f * fx);
    fy = fy * fy * (3.0f - 2.0f * fy);

    auto hash = [](int x, int y) -> float {
        int n = x * 374761393 + y * 668265263;
        n = (n ^ (n >> 13)) * 1274126177;
        return static_cast<float>((n ^ (n >> 16)) & 0x7FFFFFFF) / static_cast<float>(0x7FFFFFFF);
    };

    float a = hash(ix, iy);
    float b = hash(ix + 1, iy);
    float c = hash(ix, iy + 1);
    float d = hash(ix + 1, iy + 1);

    return a * (1 - fx) * (1 - fy) + b * fx * (1 - fy) + c * (1 - fx) * fy + d * fx * fy;
}

float FractalNoiseEffect::fractalNoise(float x, float y, int octaves, float lacunarity, float gain) const {
    float sum = 0;
    float amp = 1.0f;
    float freq = 1.0f;
    float maxAmp = 0;
    for (int i = 0; i < octaves; i++) {
        sum += noise2D(x * freq, y * freq) * amp;
        maxAmp += amp;
        amp *= gain;
        freq *= lacunarity;
    }
    return sum / maxAmp;
}

void FractalNoiseEffect::render(PixelBuffer& buffer, double time) {
    float contrast = getFloatParam("contrast") / 100.0f;
    float brightness = getFloatParam("brightness") / 100.0f;
    int complexity = getIntParam("complexity");
    int seed = getIntParam("seed");
    float offset = getFloatParam("offset");
    float scale = getFloatParam("scale") / 100.0f;
    int type = getDropdownParam("type");

    float seedF = seed * 0.1f;

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            float nx = (x / scale + seedF) * 0.01f;
            float ny = (y / scale + seedF) * 0.01f;
            float val;
            if (type == 0) {
                val = fractalNoise(nx + offset * 0.001f, ny + offset * 0.001f, complexity, 2.0f, 0.5f);
            } else if (type == 1) {
                val = std::abs(fractalNoise(nx + offset * 0.001f, ny + offset * 0.001f, complexity, 2.0f, 0.5f) * 2.0f - 1.0f);
            } else {
                val = fractalNoise(nx + time * 0.01f, ny + offset * 0.001f, complexity, 2.0f, 0.5f);
            }
            val = std::clamp((val - 0.5f) * contrast + 0.5f + brightness, 0.0f, 1.0f);
            uint8_t gray = static_cast<uint8_t>(val * 255.0f);
            uint8_t* p = buffer.pixelAt(x, y);
            p[0] = gray; p[1] = gray; p[2] = gray;
        }
    }
}

} // namespace FreeEffect
