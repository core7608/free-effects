#include "../effect_registry.h"
#include "turbulent_displace_effect.h"
#include <cmath>
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<TurbulentDisplaceEffect> s_reg("Turbulent Displace", "Distort");

TurbulentDisplaceEffect::TurbulentDisplaceEffect() {
    addParameter(EffectParameter::makeFloat("amount", "Amount", -500.0, 500.0, 50.0));
    addParameter(EffectParameter::makeFloat("size", "Size", 1.0, 500.0, 50.0));
    addParameter(EffectParameter::makeFloat("complexity", "Complexity", 1.0, 10.0, 3.0));
    addParameter(EffectParameter::makeFloat("offsetTurbulence", "Offset Turbulence", 0.0, 10000.0, 0.0));
}

std::unique_ptr<Effect> TurbulentDisplaceEffect::clone() const {
    auto e = std::make_unique<TurbulentDisplaceEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

double TurbulentDisplaceEffect::noise2d(double x, double y) const {
    int ix = static_cast<int>(std::floor(x));
    int iy = static_cast<int>(std::floor(y));
    double fx = x - ix;
    double fy = y - iy;

    auto hash = [](int x, int y) -> double {
        int h = x * 374761393 + y * 668265263;
        h = (h ^ (h >> 13)) * 1274126177;
        return static_cast<double>((h ^ (h >> 16)) & 0x7FFFFFFF) / 2147483647.0;
    };

    double v00 = hash(ix, iy);
    double v10 = hash(ix + 1, iy);
    double v01 = hash(ix, iy + 1);
    double v11 = hash(ix + 1, iy + 1);

    double tx = fx * fx * (3.0 - 2.0 * fx);
    double ty = fy * fy * (3.0 - 2.0 * fy);

    double a = v00 + (v10 - v00) * tx;
    double b = v01 + (v11 - v01) * tx;
    return a + (b - a) * ty;
}

double TurbulentDisplaceEffect::fractalNoise(double x, double y, int octaves) const {
    double val = 0.0;
    double amp = 1.0;
    double freq = 1.0;
    double maxVal = 0.0;
    for (int i = 0; i < octaves; i++) {
        val += noise2d(x * freq, y * freq) * amp;
        maxVal += amp;
        amp *= 0.5;
        freq *= 2.0;
    }
    return val / maxVal;
}

void TurbulentDisplaceEffect::render(PixelBuffer& buffer, double time) {
    if (buffer.width == 0 || buffer.height == 0) return;

    double amount = getFloatParam("amount");
    double size = getFloatParam("size");
    int octaves = static_cast<int>(getFloatParam("complexity"));
    double offset = getFloatParam("offsetTurbulence") + time * 10.0;
    double scale = 1.0 / std::max(size, 1.0);

    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            double nx = x * scale + offset * 0.1;
            double ny = y * scale + offset * 0.1;

            double dx = (fractalNoise(nx, ny, octaves) - 0.5) * amount;
            double dy = (fractalNoise(nx + 100.0, ny + 100.0, octaves) - 0.5) * amount;

            int sx = std::clamp(static_cast<int>(x + dx), 0, buffer.width - 1);
            int sy = std::clamp(static_cast<int>(y + dy), 0, buffer.height - 1);
            const uint8_t* src = buffer.pixelAt(sx, sy);
            uint8_t* dst = tmp.pixelAt(x, y);
            dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3];
        }
    }
    buffer.data = tmp.data;
}

} // namespace FreeEffect
