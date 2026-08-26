#include "../effect_registry.h"
#include "wind_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<WindEffect> s_reg("Wind", "Stylize");

WindEffect::WindEffect() {
    addParameter(EffectParameter::makeInt("windStrength", "Wind Strength", 1, 100, 10));
    addParameter(EffectParameter::makeDropdown("direction", "Direction", {"From Left", "From Right"}, 0));
    addParameter(EffectParameter::makeDropdown("method", "Method", {"Wind", "Blast"}, 0));
}

std::unique_ptr<Effect> WindEffect::clone() const {
    auto e = std::make_unique<WindEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void WindEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    int strength = getIntParam("windStrength");
    bool fromRight = (getDropdownParam("direction") == 1);
    bool blast = (getDropdownParam("method") == 1);

    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);
    std::copy(buffer.data.begin(), buffer.data.end(), tmp.data.begin());

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            const uint8_t* p = tmp.pixelAt(x, y);
            float luma = (0.299f * p[0] + 0.587f * p[1] + 0.114f * p[2]) / 255.0f;
            if (luma > 0.3f) {
                int streakLen = static_cast<int>(luma * strength * (blast ? 3 : 1));
                for (int s = 0; s < streakLen; s++) {
                    int sx = fromRight ? x - s : x + s;
                    if (sx < 0 || sx >= buffer.width) break;
                    uint8_t* dst = buffer.pixelAt(sx, y);
                    float fade = 1.0f - static_cast<float>(s) / streakLen;
                    dst[0] = static_cast<uint8_t>(std::min(255.0f, p[0] * fade + dst[0] * (1.0f - fade)));
                    dst[1] = static_cast<uint8_t>(std::min(255.0f, p[1] * fade + dst[1] * (1.0f - fade)));
                    dst[2] = static_cast<uint8_t>(std::min(255.0f, p[2] * fade + dst[2] * (1.0f - fade)));
                }
            }
        }
    }
}

} // namespace FreeEffect
