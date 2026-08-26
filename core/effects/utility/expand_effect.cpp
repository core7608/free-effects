#include "../effect_registry.h"
#include "expand_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<ExpandEffect> s_reg("Expand", "Utility");

ExpandEffect::ExpandEffect() {
    addParameter(EffectParameter::makeInt("expand", "Expand", -100, 100, 0));
    addParameter(EffectParameter::makeFloat("feather", "Feather", 0.0, 100.0, 0.0));
}

std::unique_ptr<Effect> ExpandEffect::clone() const {
    auto e = std::make_unique<ExpandEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void ExpandEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    int expand = getIntParam("expand");
    float feather = getFloatParam("feather") / 100.0f;

    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);
    std::copy(buffer.data.begin(), buffer.data.end(), tmp.data.begin());

    int radius = std::abs(expand);
    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            const uint8_t* p = tmp.pixelAt(x, y);
            float origA = p[3] / 255.0f;
            float newA = origA;

            if (radius > 0) {
                float bestA = expand > 0 ? 0.0f : 1.0f;
                for (int dy = -radius; dy <= radius; dy++) {
                    for (int dx = -radius; dx <= radius; dx++) {
                        int sx = std::clamp(x+dx, 0, buffer.width-1);
                        int sy = std::clamp(y+dy, 0, buffer.height-1);
                        float na = tmp.pixelAt(sx, sy)[3] / 255.0f;
                        if (expand > 0) bestA = std::max(bestA, na);
                        else bestA = std::min(bestA, na);
                    }
                }
                newA = bestA;
            }
            if (feather > 0) {
                float blurA = 0; int count = 0;
                int fRad = static_cast<int>(feather * 10);
                for (int dy = -fRad; dy <= fRad; dy++) {
                    for (int dx = -fRad; dx <= fRad; dx++) {
                        int sx = std::clamp(x+dx, 0, buffer.width-1);
                        int sy = std::clamp(y+dy, 0, buffer.height-1);
                        blurA += tmp.pixelAt(sx, sy)[3] / 255.0f; count++;
                    }
                }
                newA = newA * (1.0f - feather) + (blurA / count) * feather;
            }
            uint8_t* dst = buffer.pixelAt(x, y);
            dst[3] = static_cast<uint8_t>(std::clamp(newA * 255.0f, 0.0f, 255.0f));
        }
    }
}

} // namespace FreeEffect
