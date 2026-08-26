#include "../effect_registry.h"
#include "cc_glass_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<CCGlassEffect> s_reg("CC Glass", "Stylize");

CCGlassEffect::CCGlassEffect() {
    addParameter(EffectParameter::makeFloat("softness", "Softness", 0.0, 100.0, 5.0));
    addParameter(EffectParameter::makeFloat("height", "Height", 0.0, 100.0, 50.0));
    addParameter(EffectParameter::makeFloat("displacement", "Displacement", -200.0, 200.0, 10.0));
}

std::vector<ParameterGroup> CCGlassEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeFloat("softness", "Softness", 0.0, 100.0, false),
        EffectParameter::makeFloat("height", "Height", 0.0, 100.0, false),
        EffectParameter::makeFloat("displacement", "Displacement", -200.0, 200.0, false)
    }}};
}

std::unique_ptr<Effect> CCGlassEffect::clone() const {
    auto e = std::make_unique<CCGlassEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void CCGlassEffect::render(PixelBuffer& buffer, double time) {
    float softness = getFloatParam("softness") / 100.0f;
    float height = getFloatParam("height") / 100.0f;
    float displacement = getFloatParam("displacement");
    int radius = std::max(1, static_cast<int>(softness * 5));

    PixelBuffer heightMap;
    heightMap.resize(buffer.width, buffer.height);

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            const uint8_t* p = buffer.pixelAt(x, y);
            float lum = (p[0] * 0.299f + p[1] * 0.587f + p[2] * 0.114f) / 255.0f;
            uint8_t* dst = heightMap.pixelAt(x, y);
            dst[0] = dst[1] = dst[2] = static_cast<uint8_t>(lum * height * 255.0f);
        }
    }

    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            float dx = 0, dy = 0;
            const uint8_t* h = heightMap.pixelAt(
                std::clamp(x, 0, buffer.width - 1), std::clamp(y, 0, buffer.height - 1));
            float hCenter = h[0] / 255.0f;

            if (x > 0 && x < buffer.width - 1) {
                const uint8_t* hl = heightMap.pixelAt(x - 1, y);
                const uint8_t* hr = heightMap.pixelAt(x + 1, y);
                dx = (hr[0] - hl[0]) / 255.0f;
            }
            if (y > 0 && y < buffer.height - 1) {
                const uint8_t* ht = heightMap.pixelAt(x, y - 1);
                const uint8_t* hb = heightMap.pixelAt(x, y + 1);
                dy = (hb[0] - ht[0]) / 255.0f;
            }

            int sx = static_cast<int>(x + dx * displacement);
            int sy = static_cast<int>(y + dy * displacement);
            sx = std::clamp(sx, 0, buffer.width - 1);
            sy = std::clamp(sy, 0, buffer.height - 1);
            const uint8_t* src = buffer.pixelAt(sx, sy);
            uint8_t* dst = tmp.pixelAt(x, y);

            float highlight = std::max(dx * 0.5f + dy * 0.5f, 0.0f) * height;
            dst[0] = static_cast<uint8_t>(std::clamp(static_cast<double>(src[0] + highlight * 128.0f), 0.0, 255.0));
            dst[1] = static_cast<uint8_t>(std::clamp(static_cast<double>(src[1] + highlight * 128.0f), 0.0, 255.0));
            dst[2] = static_cast<uint8_t>(std::clamp(static_cast<double>(src[2] + highlight * 128.0f), 0.0, 255.0));
            dst[3] = src[3];
        }
    }
    buffer.data = tmp.data;
}

} // namespace FreeEffect
