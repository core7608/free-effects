#include "../effect_registry.h"
#include "cc_mr_smoothie_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<CCMrSmoothieEffect> s_reg("CC Mr Smoothie", "Stylize");

CCMrSmoothieEffect::CCMrSmoothieEffect() {
    addParameter(EffectParameter::makeFloat("smoothness", "Smoothness", 0.0, 100.0, 20.0));
    addParameter(EffectParameter::makeFloat("detail", "Detail", 0.0, 100.0, 50.0));
    addParameter(EffectParameter::makeFloat("sharpness", "Sharpness", 0.0, 100.0, 50.0));
    addParameter(EffectParameter::makeInt("iterations", "Iterations", 1, 10, 3));
}

std::unique_ptr<Effect> CCMrSmoothieEffect::clone() const {
    auto e = std::make_unique<CCMrSmoothieEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void CCMrSmoothieEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    int iters = getIntParam("iterations");
    float smooth = getFloatParam("smoothness") / 100.0f;
    int radius = static_cast<int>(smooth * 10);

    for (int iter = 0; iter < iters; iter++) {
        PixelBuffer tmp;
        tmp.resize(buffer.width, buffer.height);
        std::copy(buffer.data.begin(), buffer.data.end(), tmp.data.begin());
        for (int y = 0; y < buffer.height; y++) {
            for (int x = 0; x < buffer.width; x++) {
                float rS = 0, gS = 0, bS = 0, wS = 0;
                for (int dy = -radius; dy <= radius; dy++) {
                    for (int dx = -radius; dx <= radius; dx++) {
                        int sx = std::clamp(x+dx, 0, buffer.width-1);
                        int sy = std::clamp(y+dy, 0, buffer.height-1);
                        const uint8_t* p = tmp.pixelAt(sx, sy);
                        float w = 1.0f / (1.0f + std::sqrt(float(dx*dx+dy*dy)));
                        rS += p[0]*w; gS += p[1]*w; bS += p[2]*w; wS += w;
                    }
                }
                uint8_t* dst = buffer.pixelAt(x, y);
                dst[0] = static_cast<uint8_t>(std::clamp(rS/wS, 0.0f, 255.0f));
                dst[1] = static_cast<uint8_t>(std::clamp(gS/wS, 0.0f, 255.0f));
                dst[2] = static_cast<uint8_t>(std::clamp(bS/wS, 0.0f, 255.0f));
            }
        }
    }
}

} // namespace FreeEffect
