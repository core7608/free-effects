#include "../effect_registry.h"
#include "refine_hard_matte_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<RefineHardMatteEffect> s_reg("Refine Hard Matte", "Keying");

RefineHardMatteEffect::RefineHardMatteEffect() {
    addParameter(EffectParameter::makeFloat("edgeThin", "Edge Thin", -20.0, 20.0, 0.0));
    addParameter(EffectParameter::makeFloat("edgeFeather", "Edge Feather", 0.0, 100.0, 0.0));
    addParameter(EffectParameter::makeFloat("reduceChatter", "Reduce Chatter", 0.0, 100.0, 0.0));
}

std::unique_ptr<Effect> RefineHardMatteEffect::clone() const {
    auto e = std::make_unique<RefineHardMatteEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void RefineHardMatteEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    int thin = static_cast<int>(getFloatParam("edgeThin"));
    float feather = getFloatParam("edgeFeather") / 100.0f;

    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);
    std::copy(buffer.data.begin(), buffer.data.end(), tmp.data.begin());

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            const uint8_t* p = tmp.pixelAt(x, y);
            uint8_t* dst = buffer.pixelAt(x, y);
            float a = p[3] / 255.0f;
            float edge = 0;
            for (int dy = -1; dy <= 1; dy++) for (int dx = -1; dx <= 1; dx++) {
                int sx = std::clamp(x + dx, 0, buffer.width - 1);
                int sy = std::clamp(y + dy, 0, buffer.height - 1);
                edge += tmp.pixelAt(sx, sy)[3] / 255.0f;
            }
            edge /= 9.0f;
            float diff = edge - a;
            a += thin * 0.1f * diff;
            a = std::clamp(a + feather * (edge - a), 0.0f, 1.0f);
            dst[3] = static_cast<uint8_t>(a * 255.0f);
        }
    }
}

} // namespace FreeEffect
