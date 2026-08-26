#include "../effect_registry.h"
#include "matrix_rain_effect.h"
#include <cmath>
#include <vector>
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<MatrixRainEffect> s_reg("Matrix Rain", "Simulation");

MatrixRainEffect::MatrixRainEffect() {
    addParameter(EffectParameter::makeFloat("speed", "Speed", 0.1, 5.0, 1.5));
    addParameter(EffectParameter::makeFloat("density", "Density", 0.0, 1.0, 0.7));
    addParameter(EffectParameter::makeColor("color", "Color", Color{0.0, 1.0, 0.0, 1.0}));
    addParameter(EffectParameter::makeFloat("fade", "Fade", 0.0, 1.0, 0.7));
    addParameter(EffectParameter::makeInt("column_width", "Column Width", 4, 30, 10));
}

std::vector<ParameterGroup> MatrixRainEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeFloat("speed", "Speed", 0.1, 5.0, 1.5),
        EffectParameter::makeFloat("density", "Density", 0.0, 1.0, 0.7),
        EffectParameter::makeColor("color", "Color", Color{0.0, 1.0, 0.0, 1.0}),
        EffectParameter::makeFloat("fade", "Fade", 0.0, 1.0, 0.7),
        EffectParameter::makeInt("column_width", "Column Width", 4, 30, 10)
    }}};
}

std::unique_ptr<Effect> MatrixRainEffect::clone() const {
    auto e = std::make_unique<MatrixRainEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void MatrixRainEffect::render(PixelBuffer& buffer, double time) {
    if (buffer.width == 0 || buffer.height == 0) return;
    double spd = getFloatParam("speed");
    double density = getFloatParam("density");
    Color mc = getColorParam("color");
    double fadeAmt = getFloatParam("fade");
    int colW = getIntParam("column_width");
    double cr = mc.r * 255.0;
    double cg = mc.g * 255.0;
    double cb = mc.b * 255.0;
    int cols = (buffer.width + colW - 1) / colW;
    for (int c = 0; c < cols; c++) {
        double seed = std::sin(c * 127.1 + 311.7) * 43758.5453;
        double f = seed - std::floor(seed);
        if (f > density) continue;
        double seed2 = std::sin(c * 269.5 + 183.3) * 43758.5453;
        double f2 = seed2 - std::floor(seed2);
        double speed = (0.5 + f2 * 0.5) * spd * 200.0;
        double streamLen = buffer.height * (0.3 + f2 * 0.7);
        double headY = std::fmod(time * speed + f * buffer.height * 3.0, buffer.height + streamLen) - streamLen;
        int colX = c * colW;
        for (int dy = 0; dy < streamLen; dy++) {
            int py = static_cast<int>(std::round(headY + dy));
            if (py < 0 || py >= buffer.height) continue;
            double posRatio = dy / streamLen;
            double bright = (1.0 - posRatio) * fadeAmt + (1.0 - fadeAmt);
            double charSeed = std::sin(static_cast<double>(py) * 0.3 + time * 2.0 + c * 17.3) * 43758.5453;
            double charFrac = charSeed - std::floor(charSeed);
            bright *= (0.5 + charFrac * 0.5);
            for (int dx = 0; dx < colW; dx++) {
                int px = colX + dx;
                if (px < 0 || px >= buffer.width) continue;
                uint8_t* p = buffer.pixelAt(px, py);
                double sa = p[3] / 255.0;
                double fa = bright * mc.a;
                double outA = sa + fa * (1.0 - sa);
                if (outA > 0.001) {
                    p[0] = static_cast<uint8_t>(std::clamp((p[0] * sa + cr * fa * (1.0 - sa)) / outA, 0.0, 255.0));
                    p[1] = static_cast<uint8_t>(std::clamp((p[1] * sa + cg * fa * (1.0 - sa)) / outA, 0.0, 255.0));
                    p[2] = static_cast<uint8_t>(std::clamp((p[2] * sa + cb * fa * (1.0 - sa)) / outA, 0.0, 255.0));
                }
                p[3] = static_cast<uint8_t>(std::clamp(outA * 255.0, 0.0, 255.0));
            }
        }
    }
}

} // namespace FreeEffect
