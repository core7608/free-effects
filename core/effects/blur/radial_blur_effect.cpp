#include "../effect_registry.h"
#include "radial_blur_effect.h"
#include <cmath>
#include <vector>

namespace FreeEffect {

static EffectRegistrar<RadialBlurEffect> s_reg("Radial Blur", "Blur & Sharpen");

RadialBlurEffect::RadialBlurEffect() {
    addParameter(EffectParameter::makeVec2("center", "Center", {0.5, 0.5}));
    addParameter(EffectParameter::makeFloat("amount", "Amount", 0.0, 100.0, 10.0));
    addParameter(EffectParameter::makeDropdown("type", "Type", {"Spin", "Zoom"}, 0));
}

std::vector<ParameterGroup> RadialBlurEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeVec2("center", "Center", {0.5, 0.5}),
        EffectParameter::makeFloat("amount", "Amount", 0.0, 100.0, false),
        EffectParameter::makeDropdown("type", "Type", {"Spin", "Zoom"}, 0)
    }}};
}

std::unique_ptr<Effect> RadialBlurEffect::clone() const {
    auto e = std::make_unique<RadialBlurEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void RadialBlurEffect::render(PixelBuffer& buffer, double time) {
    float amount = getFloatParam("amount") / 100.0f;
    int type = getDropdownParam("type");
    Vec2 center = getVec2Param("center");
    int cx = static_cast<int>(center.x * buffer.width);
    int cy = static_cast<int>(center.y * buffer.height);
    int samples = static_cast<int>(amount * 8) + 1;
    if (samples <= 1) return;

    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);
    std::fill(tmp.data.begin(), tmp.data.end(), 0);

    float angleRad = amount * 0.5f;

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            float r = 0, g = 0, b = 0, a = 0;
            for (int s = 0; s < samples; s++) {
                float t = static_cast<float>(s) / (samples - 1) - 0.5f;
                int sx, sy;
                if (type == 0) {
                    float a2 = t * angleRad;
                    float cosA = std::cos(a2);
                    float sinA = std::sin(a2);
                    float dx = static_cast<float>(x - cx);
                    float dy = static_cast<float>(y - cy);
                    sx = static_cast<int>(cx + dx * cosA - dy * sinA);
                    sy = static_cast<int>(cy + dx * sinA + dy * cosA);
                } else {
                    float scale = 1.0f + t * angleRad;
                    sx = static_cast<int>(cx + (x - cx) * scale);
                    sy = static_cast<int>(cy + (y - cy) * scale);
                }
                sx = std::clamp(sx, 0, buffer.width - 1);
                sy = std::clamp(sy, 0, buffer.height - 1);
                const uint8_t* p = buffer.pixelAt(sx, sy);
                r += p[0]; g += p[1]; b += p[2]; a += p[3];
            }
            uint8_t* dst = tmp.pixelAt(x, y);
            dst[0] = static_cast<uint8_t>(std::clamp(static_cast<double>(r / samples), 0.0, 255.0));
            dst[1] = static_cast<uint8_t>(std::clamp(static_cast<double>(g / samples), 0.0, 255.0));
            dst[2] = static_cast<uint8_t>(std::clamp(static_cast<double>(b / samples), 0.0, 255.0));
            dst[3] = static_cast<uint8_t>(std::clamp(static_cast<double>(a / samples), 0.0, 255.0));
        }
    }
    buffer.data = tmp.data;
}

} // namespace FreeEffect
