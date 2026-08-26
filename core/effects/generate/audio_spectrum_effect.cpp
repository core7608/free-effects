#include "../effect_registry.h"
#include "audio_spectrum_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<AudioSpectrumEffect> s_reg("Audio Spectrum", "Generate");

AudioSpectrumEffect::AudioSpectrumEffect() {
    addParameter(EffectParameter::makeString("audioLayer", "Audio Layer", ""));
    addParameter(EffectParameter::makeVec2("startPoint", "Start Point", {0.0, 0.5}));
    addParameter(EffectParameter::makeVec2("endPoint", "End Point", {1.0, 0.5}));
    addParameter(EffectParameter::makeInt("bands", "Maximum Height", 1, 1000, 200));
    addParameter(EffectParameter::makeInt("numBands", "Frequency Bands", 4, 512, 64));
    addParameter(EffectParameter::makeColor("color", "Color", {255.0, 0.0, 0.0, 1.0}));
}

std::vector<ParameterGroup> AudioSpectrumEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeString("audioLayer", "Audio Layer", ""),
        EffectParameter::makeVec2("startPoint", "Start Point", {0.0, 0.5}),
        EffectParameter::makeVec2("endPoint", "End Point", {1.0, 0.5}),
        EffectParameter::makeInt("bands", "Maximum Height", 1, 1000, false),
        EffectParameter::makeInt("numBands", "Frequency Bands", 4, 512, false),
        EffectParameter::makeColor("color", "Color", {255.0, 0.0, 0.0, 1.0})
    }}};
}

std::unique_ptr<Effect> AudioSpectrumEffect::clone() const {
    auto e = std::make_unique<AudioSpectrumEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void AudioSpectrumEffect::render(PixelBuffer& buffer, double time) {
    Vec2 sp = getVec2Param("startPoint");
    Vec2 ep = getVec2Param("endPoint");
    float maxH = getFloatParam("bands");
    int numBands = getIntParam("numBands");
    Color c = getColorParam("color");

    float sx = sp.x * buffer.width;
    float sy = sp.y * buffer.height;
    float ex = ep.x * buffer.width;
    float ey = ep.y * buffer.height;
    float dx = ex - sx;
    float dy = ey - sy;
    float length = std::sqrt(dx * dx + dy * dy);
    if (length < 1) return;
    float nx = dx / length;
    float ny = dy / length;
    float px = -ny;
    float py = nx;

    for (int band = 0; band < numBands; band++) {
        float t = static_cast<float>(band) / (numBands - 1);
        float val = std::abs(std::sin(t * 3.14159f * 2.0f + time)) * 0.5f +
                    std::abs(std::cos(t * 3.14159f * 3.0f + time * 0.7f)) * 0.3f;
        float barH = val * maxH;
        float cx2 = sx + dx * t;
        float cy2 = sy + dy * t;

        for (int h = 0; h < static_cast<int>(barH); h++) {
            float bx = cx2 + px * h;
            float by = cy2 + py * h;
            int ix = static_cast<int>(bx);
            int iy = static_cast<int>(by);
            if (ix >= 0 && ix < buffer.width && iy >= 0 && iy < buffer.height) {
                uint8_t* p = buffer.pixelAt(ix, iy);
                float bright = 1.0f - static_cast<float>(h) / std::max(barH, 1.0f);
                p[0] = static_cast<uint8_t>(std::clamp(static_cast<double>(c.r * bright), 0.0, 255.0));
                p[1] = static_cast<uint8_t>(std::clamp(static_cast<double>(c.g * bright), 0.0, 255.0));
                p[2] = static_cast<uint8_t>(std::clamp(static_cast<double>(c.b * bright), 0.0, 255.0));
                p[3] = static_cast<uint8_t>(std::clamp(static_cast<double>(bright * 255.0f), 0.0, 255.0));
            }
        }
    }
}

} // namespace FreeEffect
