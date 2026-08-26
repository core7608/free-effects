#include "../effect_registry.h"
#include "glitch_transition.h"
#include <cmath>
#include <vector>
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<GlitchTransition> s_reg("Glitch Transition", "Transition");

GlitchTransition::GlitchTransition() {
    addParameter(EffectParameter::makeFloat("progress", "Progress", 0.0, 1.0, 0.0));
    addParameter(EffectParameter::makeFloat("intensity", "Intensity", 0.0, 1.0, 0.7));
    addParameter(EffectParameter::makeInt("slices", "Slice Count", 1, 50, 15));
    addParameter(EffectParameter::makeBool("rgb_shift", "RGB Shift", true));
}

std::vector<ParameterGroup> GlitchTransition::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeFloat("progress", "Progress", 0.0, 1.0, 0.0),
        EffectParameter::makeFloat("intensity", "Intensity", 0.0, 1.0, 0.7),
        EffectParameter::makeInt("slices", "Slice Count", 1, 50, 15),
        EffectParameter::makeBool("rgb_shift", "RGB Shift", true)
    }}};
}

std::unique_ptr<Effect> GlitchTransition::clone() const {
    auto e = std::make_unique<GlitchTransition>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void GlitchTransition::render(PixelBuffer& buffer, double time) {
    if (buffer.width == 0 || buffer.height == 0) return;
    double progress = getFloatParam("progress");
    double intensity = getFloatParam("intensity");
    int slices = getIntParam("slices");
    bool rgbShift = getBoolParam("rgb_shift");
    double glitchStrength = std::sin(progress * 3.14159) * intensity;
    if (glitchStrength < 0.01) return;
    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);
    tmp.data = buffer.data;
    int sliceH = buffer.height / slices;
    for (int s = 0; s < slices; s++) {
        double seed = std::sin(s * 127.1 + time * 17.3) * 43758.5453;
        double f = seed - std::floor(seed);
        if (f > glitchStrength * 2.0) continue;
        int shiftX = static_cast<int>((f - 0.5) * 80.0 * glitchStrength);
        int yStart = s * sliceH;
        int yEnd = std::min((s + 1) * sliceH, buffer.height);
        for (int y = yStart; y < yEnd; y++) {
            for (int x = 0; x < buffer.width; x++) {
                int sx = std::clamp(x + shiftX, 0, buffer.width - 1);
                const uint8_t* src = tmp.pixelAt(sx, y);
                uint8_t* dst = buffer.pixelAt(x, y);
                if (rgbShift) {
                    int rShift = std::clamp(x + shiftX + static_cast<int>(glitchStrength * 8), 0, buffer.width - 1);
                    int bShift = std::clamp(x + shiftX - static_cast<int>(glitchStrength * 8), 0, buffer.width - 1);
                    const uint8_t* rp = tmp.pixelAt(rShift, y);
                    const uint8_t* bp = tmp.pixelAt(bShift, y);
                    dst[0] = rp[0];
                    dst[1] = src[1];
                    dst[2] = bp[2];
                } else {
                    dst[0] = src[0];
                    dst[1] = src[1];
                    dst[2] = src[2];
                }
                dst[3] = src[3];
            }
        }
    }
    for (int i = 0; i < 20 * glitchStrength; i++) {
        double seed = std::sin(i * 453.1 + time * 291.7) * 43758.5453;
        double f = seed - std::floor(seed);
        int rx = static_cast<int>(f * buffer.width);
        double seed2 = std::sin(i * 713.3 + time * 157.9) * 43758.5453;
        double f2 = seed2 - std::floor(seed2);
        int ry = static_cast<int>(f2 * buffer.height);
        int rw = static_cast<int>((1.0 + f * 3.0) * glitchStrength * 5.0);
        int rh = static_cast<int>((1.0 + f2 * 2.0) * glitchStrength * 3.0);
        double rv = f * 255.0;
        for (int dy = 0; dy < rh && ry + dy < buffer.height; dy++) {
            for (int dx = 0; dx < rw && rx + dx < buffer.width; dx++) {
                uint8_t* p = buffer.pixelAt(rx + dx, ry + dy);
                p[0] = static_cast<uint8_t>(rv);
                p[1] = static_cast<uint8_t>(rv * 0.8);
                p[2] = static_cast<uint8_t>(rv * 0.9);
            }
        }
    }
    double fade = std::abs(std::sin(progress * 3.14159));
    for (int i = 0; i < buffer.width * buffer.height; i++) {
        uint8_t* p = buffer.data.data() + i * 4;
        p[3] = static_cast<uint8_t>(p[3] * (1.0 - fade * 0.2));
    }
}

} // namespace FreeEffect
