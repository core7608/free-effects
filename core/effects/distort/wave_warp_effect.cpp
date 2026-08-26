#include "../effect_registry.h"
#include "wave_warp_effect.h"
#include <cmath>
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<WaveWarpEffect> s_reg("Wave Warp", "Distort");

WaveWarpEffect::WaveWarpEffect() {
    addParameter(EffectParameter::makeDropdown("waveType", "Wave Type", {"Sine", "Square", "Triangle"}, 0));
    addParameter(EffectParameter::makeFloat("height", "Wave Height", 0.0, 500.0, 10.0));
    addParameter(EffectParameter::makeFloat("width", "Wave Width", 1.0, 500.0, 40.0));
    addParameter(EffectParameter::makeAngle("direction", "Direction", 0.0));
    addParameter(EffectParameter::makeFloat("speed", "Wave Speed", -10.0, 10.0, 1.0));
}

std::vector<ParameterGroup> WaveWarpEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeDropdown("waveType", "Wave Type", {"Sine", "Square", "Triangle"}, 0),
        EffectParameter::makeFloat("height", "Wave Height", 0.0, 500.0, false),
        EffectParameter::makeFloat("width", "Wave Width", 1.0, 500.0, false),
        EffectParameter::makeAngle("direction", "Direction", 0.0),
        EffectParameter::makeFloat("speed", "Wave Speed", -10.0, 10.0, false)
    }}};
}

std::unique_ptr<Effect> WaveWarpEffect::clone() const {
    auto e = std::make_unique<WaveWarpEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void WaveWarpEffect::render(PixelBuffer& buffer, double time) {
    int waveType = getDropdownParam("waveType");
    float height = getFloatParam("height");
    float width = getFloatParam("width");
    float dir = getAngleParam("direction") * 3.14159265f / 180.0f;
    float speed = getFloatParam("speed");

    float dx = std::cos(dir);
    float dy = std::sin(dir);
    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            float phase = (x * dx + y * dy) / std::max(width, 1.0f) + time * speed;
            float wave;
            switch (waveType) {
                case 0: wave = std::sin(phase * 2.0f * 3.14159265f); break;
                case 1: wave = std::sin(phase * 2.0f * 3.14159265f) > 0 ? 1.0f : -1.0f; break;
                case 2: wave = std::abs(std::fmod(phase * 4.0f, 4.0f) - 2.0f) - 1.0f; break;
                default: wave = 0; break;
            }
            int sx = static_cast<int>(x + wave * height * dy);
            int sy = static_cast<int>(y - wave * height * dx);
            sx = std::clamp(sx, 0, buffer.width - 1);
            sy = std::clamp(sy, 0, buffer.height - 1);
            const uint8_t* src = buffer.pixelAt(sx, sy);
            uint8_t* dst = tmp.pixelAt(x, y);
            dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3];
        }
    }
    buffer.data = tmp.data;
}

} // namespace FreeEffect
