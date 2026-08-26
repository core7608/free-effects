#include "../effect_registry.h"
#include "sharpen_effect.h"
#include <algorithm>
#include <vector>

namespace FreeEffect {

static EffectRegistrar<SharpenEffect> s_reg("Sharpen", "Blur & Sharpen");

SharpenEffect::SharpenEffect() {
    addParameter(EffectParameter::makeFloat("amount", "Amount", 0.0, 100.0, 50.0));
}

std::vector<ParameterGroup> SharpenEffect::getParameterGroups() const {
    return {{getName(), {EffectParameter::makeFloat("amount", "Amount", 0.0, 100.0, false)}}};
}

std::unique_ptr<Effect> SharpenEffect::clone() const {
    auto e = std::make_unique<SharpenEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void SharpenEffect::render(PixelBuffer& buffer, double time) {
    float amount = getFloatParam("amount") / 100.0f * 4.0f;
    static const float kernel[9] = {
        0, -1, 0,
        -1, 5, -1,
        0, -1, 0
    };

    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            float r = 0, g = 0, b = 0;
            int ki = 0;
            for (int ky = -1; ky <= 1; ky++) {
                for (int kx = -1; kx <= 1; kx++) {
                    int sx = std::clamp(x + kx, 0, buffer.width - 1);
                    int sy = std::clamp(y + ky, 0, buffer.height - 1);
                    const uint8_t* p = buffer.pixelAt(sx, sy);
                    float w = 1.0f + (kernel[ki] - 1.0f) * amount;
                    r += p[0] * w;
                    g += p[1] * w;
                    b += p[2] * w;
                    ki++;
                }
            }
            uint8_t* dst = tmp.pixelAt(x, y);
            dst[0] = static_cast<uint8_t>(std::clamp(static_cast<double>(r), 0.0, 255.0));
            dst[1] = static_cast<uint8_t>(std::clamp(static_cast<double>(g), 0.0, 255.0));
            dst[2] = static_cast<uint8_t>(std::clamp(static_cast<double>(b), 0.0, 255.0));
            dst[3] = buffer.pixelAt(x, y)[3];
        }
    }
    buffer.data = tmp.data;
}

} // namespace FreeEffect
