#include "../effect_registry.h"
#include "brush_strokes_effect.h"
#include <algorithm>
#include <cmath>
#include <random>

namespace FreeEffect {

static EffectRegistrar<BrushStrokesEffect> s_reg("Brush Strokes", "Stylize");

BrushStrokesEffect::BrushStrokesEffect() {
    addParameter(EffectParameter::makeFloat("strokeLength", "Stroke Length", 1.0, 100.0, 15.0));
    addParameter(EffectParameter::makeFloat("strokeThickness", "Stroke Thickness", 1.0, 50.0, 3.0));
    addParameter(EffectParameter::makeAngle("direction", "Direction", 45.0));
}

std::vector<ParameterGroup> BrushStrokesEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeFloat("strokeLength", "Stroke Length", 1.0, 100.0, false),
        EffectParameter::makeFloat("strokeThickness", "Stroke Thickness", 1.0, 50.0, false),
        EffectParameter::makeAngle("direction", "Direction", 45.0)
    }}};
}

std::unique_ptr<Effect> BrushStrokesEffect::clone() const {
    auto e = std::make_unique<BrushStrokesEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void BrushStrokesEffect::render(PixelBuffer& buffer, double time) {
    float length = getFloatParam("strokeLength");
    float thickness = getFloatParam("strokeThickness");
    float angle = getAngleParam("direction") * 3.14159265f / 180.0f;
    float dx = std::cos(angle);
    float dy = std::sin(angle);
    int samples = static_cast<int>(length);
    int thick = static_cast<int>(thickness / 2.0f) + 1;

    std::mt19937 rng(42);
    std::uniform_real_distribution<float> dist(-0.5f, 0.5f);

    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            float r = 0, g = 0, b = 0;
            int count = 0;
            for (int s = 0; s < samples; s++) {
                float t = (static_cast<float>(s) / samples - 0.5f) * length;
                for (int th = -thick; th <= thick; th++) {
                    float ox = dist(rng) * thickness * 0.3f;
                    float oy = dist(rng) * thickness * 0.3f;
                    int sx = static_cast<int>(x + dx * t + (-dy) * th + ox);
                    int sy = static_cast<int>(y + dy * t + dx * th + oy);
                    sx = std::clamp(sx, 0, buffer.width - 1);
                    sy = std::clamp(sy, 0, buffer.height - 1);
                    const uint8_t* p = buffer.pixelAt(sx, sy);
                    r += p[0]; g += p[1]; b += p[2];
                    count++;
                }
            }
            if (count > 0) {
                uint8_t* dst = tmp.pixelAt(x, y);
                const uint8_t* src = buffer.pixelAt(x, y);
                float blend = 0.7f;
                dst[0] = static_cast<uint8_t>(src[0] * (1.0f - blend) + (r / count) * blend);
                dst[1] = static_cast<uint8_t>(src[1] * (1.0f - blend) + (g / count) * blend);
                dst[2] = static_cast<uint8_t>(src[2] * (1.0f - blend) + (b / count) * blend);
                dst[3] = src[3];
            }
        }
    }
    buffer.data = tmp.data;
}

} // namespace FreeEffect
