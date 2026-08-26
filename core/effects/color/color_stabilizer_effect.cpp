#include "../effect_registry.h"
#include "color_stabilizer_effect.h"
#include <algorithm>
#include <cmath>
#include <numeric>

namespace FreeEffect {

static EffectRegistrar<ColorStabilizerEffect> s_reg("Color Stabilizer", "Color Correction");

ColorStabilizerEffect::ColorStabilizerEffect() {
    addParameter(EffectParameter::makeVec2("regionPoint1", "Region Point 1", {0.0, 0.0}));
    addParameter(EffectParameter::makeVec2("regionPoint2", "Region Point 2", {1.0, 1.0}));
    addParameter(EffectParameter::makeFloat("smoothing", "Smoothing", 0.0, 100.0, 20.0));
    addParameter(EffectParameter::makeDropdown("mode", "Stabilization Mode", {"Brightness", "Color", "Exposure"}, 0));
}

std::unique_ptr<Effect> ColorStabilizerEffect::clone() const {
    auto e = std::make_unique<ColorStabilizerEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void ColorStabilizerEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    float smoothing = getFloatParam("smoothing") / 100.0f;
    int mode = getDropdownParam("mode");

    float totalR = 0, totalG = 0, totalB = 0;
    int count = 0;
    int sx = static_cast<int>(getVec2Param("regionPoint1").x * buffer.width);
    int sy = static_cast<int>(getVec2Param("regionPoint1").y * buffer.height);
    int ex = static_cast<int>(getVec2Param("regionPoint2").x * buffer.width);
    int ey = static_cast<int>(getVec2Param("regionPoint2").y * buffer.height);
    sx = std::clamp(sx, 0, buffer.width - 1); sy = std::clamp(sy, 0, buffer.height - 1);
    ex = std::clamp(ex, 0, buffer.width - 1); ey = std::clamp(ey, 0, buffer.height - 1);

    for (int y = sy; y <= ey; y++) {
        for (int x = sx; x <= ex; x++) {
            const uint8_t* p = buffer.pixelAt(x, y);
            totalR += p[0]; totalG += p[1]; totalB += p[2];
            count++;
        }
    }
    if (count <= 0) return;
    float avgR = totalR / count, avgG = totalG / count, avgB = totalB / count;
    float targetLuma = 128.0f;
    float currentLuma = 0.299f * avgR + 0.587f * avgG + 0.114f * avgB;

    float gainR = 1.0f, gainG = 1.0f, gainB = 1.0f;
    float offsetR = 0, offsetG = 0, offsetB = 0;

    if (mode == 0) {
        float g = targetLuma / std::max(currentLuma, 1.0f);
        gainR = gainG = gainB = smoothing * g + (1.0f - smoothing);
    } else if (mode == 1) {
        gainR = smoothing * (targetLuma / std::max(avgR, 1.0f)) + (1.0f - smoothing);
        gainG = smoothing * (targetLuma / std::max(avgG, 1.0f)) + (1.0f - smoothing);
        gainB = smoothing * (targetLuma / std::max(avgB, 1.0f)) + (1.0f - smoothing);
    } else {
        float expGain = targetLuma / std::max(currentLuma, 1.0f);
        gainR = smoothing * expGain + (1.0f - smoothing);
        gainG = gainR; gainB = gainR;
    }

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            p[0] = static_cast<uint8_t>(std::clamp(static_cast<double>(p[0] * gainR + offsetR), 0.0, 255.0));
            p[1] = static_cast<uint8_t>(std::clamp(static_cast<double>(p[1] * gainG + offsetG), 0.0, 255.0));
            p[2] = static_cast<uint8_t>(std::clamp(static_cast<double>(p[2] * gainB + offsetB), 0.0, 255.0));
        }
    }
}

} // namespace FreeEffect
