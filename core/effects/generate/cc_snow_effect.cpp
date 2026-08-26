#include "../effect_registry.h"
#include "cc_snow_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<CCSnowEffect> s_reg("CC Snow", "Generate");

CCSnowEffect::CCSnowEffect() {
    addParameter(EffectParameter::makeInt("flakes", "Flakes", 1, 500, 100));
    addParameter(EffectParameter::makeFloat("size", "Size", 1.0, 20.0, 5.0));
    addParameter(EffectParameter::makeFloat("speed", "Speed", 0.0, 200.0, 50.0));
    addParameter(EffectParameter::makeFloat("wind", "Wind", -100.0, 100.0, 0.0));
    addParameter(EffectParameter::makeColor("color", "Color", {255.0, 255.0, 255.0, 1.0}));
}

std::unique_ptr<Effect> CCSnowEffect::clone() const {
    auto e = std::make_unique<CCSnowEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void CCSnowEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    int flakes = getIntParam("flakes");
    float sz = getFloatParam("size");
    float spd = getFloatParam("speed") / 100.0f;
    float wind = getFloatParam("wind") / 100.0f;
    Color col = getColorParam("color");

    for (int i = 0; i < flakes; i++) {
        float hash1 = std::fmod(std::sin(static_cast<float>(i) * 127.1f) * 43758.5453f, 1.0f);
        float hash2 = std::fmod(std::sin(static_cast<float>(i) * 269.5f) * 43758.5453f, 1.0f);
        float x = std::fmod(hash1 * buffer.width + time * wind * 50.0f + i * 17.0f, static_cast<float>(buffer.width));
        float y = std::fmod(hash2 * buffer.height + time * spd * 100.0f + i * 23.0f, static_cast<float>(buffer.height));

        for (int dy = static_cast<int>(-sz); dy <= static_cast<int>(sz); dy++) {
            for (int dx = static_cast<int>(-sz); dx <= static_cast<int>(sz); dx++) {
                if (dx*dx+dy*dy > sz*sz) continue;
                int px = std::clamp(static_cast<int>(x + dx), 0, buffer.width - 1);
                int py = std::clamp(static_cast<int>(y + dy), 0, buffer.height - 1);
                uint8_t* p = buffer.pixelAt(px, py);
                float fade = 1.0f - std::sqrt(float(dx*dx+dy*dy)) / sz;
                p[0] = static_cast<uint8_t>(std::min(static_cast<double>(255.0f), static_cast<double>(p[0] + col.r * fade)));
                p[1] = static_cast<uint8_t>(std::min(static_cast<double>(255.0f), static_cast<double>(p[1] + col.g * fade)));
                p[2] = static_cast<uint8_t>(std::min(static_cast<double>(255.0f), static_cast<double>(p[2] + col.b * fade)));
            }
        }
    }
}

} // namespace FreeEffect
