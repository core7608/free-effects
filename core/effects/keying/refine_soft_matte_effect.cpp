#include "../effect_registry.h"
#include "refine_soft_matte_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<RefineSoftMatteEffect> s_reg("Refine Soft Matte", "Keying");

RefineSoftMatteEffect::RefineSoftMatteEffect() {
    addParameter(EffectParameter::makeFloat("radius", "Radius", 0.0, 100.0, 5.0));
    addParameter(EffectParameter::makeFloat("edgeThin", "Edge Thin", -20.0, 20.0, 0.0));
    addParameter(EffectParameter::makeFloat("featherRadius", "Feather Radius", 0.0, 100.0, 0.0));
}

std::unique_ptr<Effect> RefineSoftMatteEffect::clone() const {
    auto e = std::make_unique<RefineSoftMatteEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void RefineSoftMatteEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    int radius = static_cast<int>(getFloatParam("radius"));
    float edgeThin = getFloatParam("edgeThin") / 100.0f;

    if (radius <= 0) return;
    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);
    std::copy(buffer.data.begin(), buffer.data.end(), tmp.data.begin());

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            float aSum = 0, wSum = 0;
            for (int dy = -radius; dy <= radius; dy++) {
                for (int dx = -radius; dx <= radius; dx++) {
                    int sx = std::clamp(x + dx, 0, buffer.width - 1);
                    int sy = std::clamp(y + dy, 0, buffer.height - 1);
                    float d = std::sqrt(static_cast<float>(dx*dx + dy*dy));
                    float w = std::max(0.0f, 1.0f - d / radius);
                    aSum += tmp.pixelAt(sx, sy)[3] / 255.0f * w;
                    wSum += w;
                }
            }
            uint8_t* dst = buffer.pixelAt(x, y);
            float origA = dst[3] / 255.0f;
            float newA = (wSum > 0) ? aSum / wSum : origA;
            dst[3] = static_cast<uint8_t>(std::clamp(origA + (newA - origA) * (1.0f + edgeThin) * 255.0f, 0.0f, 255.0f));
        }
    }
}

} // namespace FreeEffect
