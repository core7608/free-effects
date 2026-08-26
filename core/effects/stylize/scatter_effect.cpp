#include "../effect_registry.h"
#include "scatter_effect.h"
#include <cmath>
#include <algorithm>
#include <random>

namespace FreeEffect {

static EffectRegistrar<ScatterEffect> s_reg("Scatter", "Stylize");

ScatterEffect::ScatterEffect() {
    addParameter(EffectParameter::makeFloat("grain", "Grain", 0.0, 100.0, 0.0));
    addParameter(EffectParameter::makeFloat("amount", "Amount", 0.0, 100.0, 30.0));
    addParameter(EffectParameter::makeBool("scatter", "Scatter", true));
    addParameter(EffectParameter::makeAngle("direction", "Direction", 0.0));
}

std::unique_ptr<Effect> ScatterEffect::clone() const {
    auto e = std::make_unique<ScatterEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void ScatterEffect::render(PixelBuffer& buffer, double time) {
    if (buffer.width == 0 || buffer.height == 0) return;

    double amount = getFloatParam("amount");
    double dir = getAngleParam("direction") * 3.14159265 / 180.0;

    if (amount <= 0.0) return;

    std::mt19937 gen(static_cast<unsigned int>(time * 1000.0 + amount));
    std::uniform_real_distribution<double> dist(-1.0, 1.0);

    double maxDisp = amount * 0.5;
    double cosD = std::cos(dir);
    double sinD = std::sin(dir);

    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            double r = dist(gen) * maxDisp;
            double dx = r * cosD;
            double dy = r * sinD;

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
