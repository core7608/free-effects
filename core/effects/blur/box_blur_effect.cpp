#include "../effect_registry.h"
#include "box_blur_effect.h"
#include <algorithm>
#include <vector>

namespace FreeEffect {

static EffectRegistrar<BoxBlurEffect> s_reg("Box Blur", "Blur & Sharpen");

BoxBlurEffect::BoxBlurEffect() {
    addParameter(EffectParameter::makeFloat("radius", "Blur Radius", 0.0, 200.0, 5.0));
    addParameter(EffectParameter::makeInt("passes", "Iterations", 1, 10, 1));
}

std::vector<ParameterGroup> BoxBlurEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeFloat("radius", "Blur Radius", 0.0, 200.0, false),
        EffectParameter::makeInt("passes", "Iterations", 1, 10, false)
    }}};
}

std::unique_ptr<Effect> BoxBlurEffect::clone() const {
    auto e = std::make_unique<BoxBlurEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void BoxBlurEffect::render(PixelBuffer& buffer, double time) {
    int radius = static_cast<int>(getFloatParam("radius"));
    int passes = getIntParam("passes");
    if (radius <= 0) return;

    for (int pass = 0; pass < passes; pass++) {
        PixelBuffer tmp;
        tmp.resize(buffer.width, buffer.height);

        for (int y = 0; y < buffer.height; y++) {
            for (int x = 0; x < buffer.width; x++) {
                float r = 0, g = 0, b = 0, a = 0;
                int count = 0;
                for (int ky = -radius; ky <= radius; ky++) {
                    for (int kx = -radius; kx <= radius; kx++) {
                        int sx = std::clamp(x + kx, 0, buffer.width - 1);
                        int sy = std::clamp(y + ky, 0, buffer.height - 1);
                        const uint8_t* p = buffer.pixelAt(sx, sy);
                        r += p[0]; g += p[1]; b += p[2]; a += p[3];
                        count++;
                    }
                }
                uint8_t* dst = tmp.pixelAt(x, y);
                dst[0] = static_cast<uint8_t>(r / count);
                dst[1] = static_cast<uint8_t>(g / count);
                dst[2] = static_cast<uint8_t>(b / count);
                dst[3] = static_cast<uint8_t>(a / count);
            }
        }
        buffer.data = tmp.data;
    }
}

} // namespace FreeEffect
