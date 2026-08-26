#include "../effect_registry.h"
#include "audio_waveform_effect.h"
#include <cmath>
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<AudioWaveformEffect> s_reg("Audio Waveform", "Generate");

AudioWaveformEffect::AudioWaveformEffect() {
    addParameter(EffectParameter::makeFloat("amplitude", "Amplitude", 0.0, 1.0, 0.5));
    addParameter(EffectParameter::makeFloat("frequency", "Frequency", 0.1, 20.0, 5.0));
    addParameter(EffectParameter::makeColor("color", "Color", Color{0.0, 1.0, 0.0, 1.0}));
    addParameter(EffectParameter::makeFloat("thickness", "Thickness", 1.0, 10.0, 2.0));
    addParameter(EffectParameter::makeFloat("offset_y", "Y Position", 0.0, 1.0, 0.5));
    addParameter(EffectParameter::makeInt("wave_type", "Wave Type", 0, 3, 0));
}

std::vector<ParameterGroup> AudioWaveformEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeFloat("amplitude", "Amplitude", 0.0, 1.0, 0.5),
        EffectParameter::makeFloat("frequency", "Frequency", 0.1, 20.0, 5.0),
        EffectParameter::makeColor("color", "Color", Color{0.0, 1.0, 0.0, 1.0}),
        EffectParameter::makeFloat("thickness", "Thickness", 1.0, 10.0, 2.0),
        EffectParameter::makeFloat("offset_y", "Y Position", 0.0, 1.0, 0.5),
        EffectParameter::makeInt("wave_type", "Wave Type", 0, 3, 0)
    }}};
}

std::unique_ptr<Effect> AudioWaveformEffect::clone() const {
    auto e = std::make_unique<AudioWaveformEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void AudioWaveformEffect::render(PixelBuffer& buffer, double time) {
    if (buffer.width == 0 || buffer.height == 0) return;
    double amp = getFloatParam("amplitude");
    double freq = getFloatParam("frequency");
    Color wc = getColorParam("color");
    double thick = getFloatParam("thickness");
    double offsetY = getFloatParam("offset_y");
    int waveType = getIntParam("wave_type");
    double baseY = offsetY * buffer.height;
    double ampPixels = amp * buffer.height * 0.4;
    for (int x = 0; x < buffer.width; x++) {
        double nx = static_cast<double>(x) / buffer.width;
        double waveVal = 0.0;
        switch (waveType) {
            case 0:
                waveVal = std::sin(nx * freq * 6.28318 + time * 3.0);
                break;
            case 1:
                waveVal = std::fmod(nx * freq + time, 1.0) < 0.5 ? 1.0 : -1.0;
                break;
            case 2: {
                double ph = std::fmod(nx * freq + time, 1.0);
                waveVal = ph < 0.5 ? ph * 4.0 - 1.0 : 3.0 - ph * 4.0;
                break;
            }
            case 3:
                waveVal = std::sin(nx * freq * 6.28318 + time * 3.0) *
                          std::sin(nx * freq * 0.3 * 6.28318 + time * 0.7);
                break;
        }
        int cy = static_cast<int>(std::round(baseY + waveVal * ampPixels));
        int halfThick = static_cast<int>(std::ceil(thick / 2.0));
        for (int dy = -halfThick; dy <= halfThick; dy++) {
            int py = cy + dy;
            if (py < 0 || py >= buffer.height) continue;
            double fade = 1.0 - static_cast<double>(std::abs(dy)) / (halfThick + 0.5);
            double fa = wc.a * fade;
            uint8_t* p = buffer.pixelAt(x, py);
            double sa = p[3] / 255.0;
            double outA = sa + fa * (1.0 - sa);
            if (outA > 0.001) {
                p[0] = static_cast<uint8_t>((p[0] * sa + wc.r * 255.0 * fa * (1.0 - sa)) / outA);
                p[1] = static_cast<uint8_t>((p[1] * sa + wc.g * 255.0 * fa * (1.0 - sa)) / outA);
                p[2] = static_cast<uint8_t>((p[2] * sa + wc.b * 255.0 * fa * (1.0 - sa)) / outA);
            }
            p[3] = static_cast<uint8_t>(std::clamp(outA * 255.0, 0.0, 255.0));
        }
    }
}

} // namespace FreeEffect
