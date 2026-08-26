#include "../../math/math_constants.h"
#include "../effect_registry.h"
#include "disintegrate_effect.h"
#include <cmath>
#include <vector>
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<DisintegrateEffect> s_reg("Disintegrate", "Simulation");

DisintegrateEffect::DisintegrateEffect() {
    addParameter(EffectParameter::makeFloat("progress", "Progress", 0.0, 1.0, 0.0));
    addParameter(EffectParameter::makeFloat("scatter", "Scatter", 0.0, 100.0, 20.0));
    addParameter(EffectParameter::makeInt("particle_size", "Particle Size", 1, 10, 3));
    addParameter(EffectParameter::makeColor("color", "Particle Color", Color{0.5, 0.5, 0.5, 0.5}));
    addParameter(EffectParameter::makeFloat("direction", "Direction", 0.0, 360.0, 0.0));
}

std::vector<ParameterGroup> DisintegrateEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeFloat("progress", "Progress", 0.0, 1.0, 0.0),
        EffectParameter::makeFloat("scatter", "Scatter", 0.0, 100.0, 20.0),
        EffectParameter::makeInt("particle_size", "Particle Size", 1, 10, 3),
        EffectParameter::makeColor("color", "Particle Color", Color{0.5, 0.5, 0.5, 0.5}),
        EffectParameter::makeFloat("direction", "Direction", 0.0, 360.0, 0.0)
    }}};
}

std::unique_ptr<Effect> DisintegrateEffect::clone() const {
    auto e = std::make_unique<DisintegrateEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void DisintegrateEffect::render(PixelBuffer& buffer, double time) {
    if (buffer.width == 0 || buffer.height == 0) return;
    double progress = getFloatParam("progress");
    double scatter = getFloatParam("scatter");
    int pSize = getIntParam("particle_size");
    Color pc = getColorParam("color");
    double dir = getFloatParam("direction") * M_PI / 180.0;
    if (progress < 0.001) return;
    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);
    tmp.data = buffer.data;
    double cr = pc.r * 255.0;
    double cg = pc.g * 255.0;
    double cb = pc.b * 255.0;
    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            double nx = static_cast<double>(x) / buffer.width;
            double ny = static_cast<double>(y) / buffer.height;
            double seed = std::sin(x * 127.1 + y * 311.7) * 43758.5453;
            double threshold = seed - std::floor(seed);
            double dissolve = std::clamp((progress - threshold * 0.8) * 2.5, 0.0, 1.0);
            if (dissolve < 0.01) {
                const uint8_t* src = tmp.pixelAt(x, y);
                uint8_t* dst = buffer.pixelAt(x, y);
                dst[0] = src[0];
                dst[1] = src[1];
                dst[2] = src[2];
                dst[3] = src[3];
                continue;
            }
            double angle = dir + (threshold - 0.5) * 0.5;
            double dist = dissolve * scatter * (0.3 + threshold * 0.7);
            int sx = static_cast<int>(std::round(x + std::cos(angle) * dist));
            int sy = static_cast<int>(std::round(y + std::sin(angle) * dist - dissolve * progress * 50.0));
            uint8_t* dst = buffer.pixelAt(x, y);
            if (sx < 0 || sx >= buffer.width || sy < 0 || sy >= buffer.height) {
                dst[0] = dst[1] = dst[2] = 0;
                dst[3] = 0;
                continue;
            }
            const uint8_t* src = tmp.pixelAt(sx, sy);
            double fade = 1.0 - dissolve * 0.5;
            double particleBright = dissolve * 0.3;
            dst[0] = static_cast<uint8_t>(std::clamp(src[0] * fade + cr * particleBright, 0.0, 255.0));
            dst[1] = static_cast<uint8_t>(std::clamp(src[1] * fade + cg * particleBright, 0.0, 255.0));
            dst[2] = static_cast<uint8_t>(std::clamp(src[2] * fade + cb * particleBright, 0.0, 255.0));
            dst[3] = static_cast<uint8_t>(src[3] * fade);
        }
    }
}

} // namespace FreeEffect
