#include "../effect_registry.h"
#include "key_cleaner_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<KeyCleanerEffect> s_reg("Key Cleaner", "Keying");

KeyCleanerEffect::KeyCleanerEffect() {
    addParameter(EffectParameter::makeFloat("edgeRadius", "Additional Edge Radius", 0.0, 100.0, 0.0));
    addParameter(EffectParameter::makeFloat("edgeThin", "Edge Thin", -100.0, 100.0, 0.0));
    addParameter(EffectParameter::makeFloat("edgeFeather", "Edge Feather", 0.0, 100.0, 0.0));
}

std::unique_ptr<Effect> KeyCleanerEffect::clone() const {
    auto e = std::make_unique<KeyCleanerEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void KeyCleanerEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    int radius = static_cast<int>(getFloatParam("edgeRadius"));
    float thin = getFloatParam("edgeThin") / 100.0f;

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            float a = p[3] / 255.0f;
            float edges = 0;
            int count = 0;
            for (int dy = -1; dy <= 1; dy++) for (int dx = -1; dx <= 1; dx++) {
                int sx = std::clamp(x + dx, 0, buffer.width - 1);
                int sy = std::clamp(y + dy, 0, buffer.height - 1);
                float na = buffer.pixelAt(sx, sy)[3] / 255.0f;
                if (std::abs(na - a) > 0.1f) edges++;
                count++;
            }
            float edgeFactor = static_cast<float>(edges) / count;
            a += thin * edgeFactor * 0.5f;
            p[3] = static_cast<uint8_t>(std::clamp(a * 255.0f, 0.0f, 255.0f));
        }
    }
}

} // namespace FreeEffect
