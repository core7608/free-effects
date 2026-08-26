#include "../effect_registry.h"
#include "cc_block_load_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<CCBlockLoadEffect> s_reg("CC Block Load", "Stylize");

CCBlockLoadEffect::CCBlockLoadEffect() {
    addParameter(EffectParameter::makeInt("blocksX", "Blocks X", 1, 50, 10));
    addParameter(EffectParameter::makeInt("blocksY", "Blocks Y", 1, 50, 10));
    addParameter(EffectParameter::makeFloat("completion", "Completion", 0.0, 100.0, 100.0));
    addParameter(EffectParameter::makeInt("randomSeed", "Random Seed", 1, 1000, 42));
}

std::unique_ptr<Effect> CCBlockLoadEffect::clone() const {
    auto e = std::make_unique<CCBlockLoadEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void CCBlockLoadEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    int bx = getIntParam("blocksX"), by = getIntParam("blocksY");
    float completion = getFloatParam("completion") / 100.0f;
    int seed = getIntParam("randomSeed");

    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);
    std::copy(buffer.data.begin(), buffer.data.end(), tmp.data.begin());

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* dst = buffer.pixelAt(x, y);
            dst[0] = dst[1] = dst[2] = 0; dst[3] = 0;
        }
    }

    for (int gy = 0; gy < by; gy++) {
        for (int gx = 0; gx < bx; gx++) {
            float hash = std::fmod(std::sin(static_cast<float>(gx * 127 + gy * 311 + seed)) * 43758.5453f, 1.0f);
            if (hash < completion) {
                int x0 = gx * buffer.width / bx, y0 = gy * buffer.height / by;
                int x1 = (gx + 1) * buffer.width / bx, y1 = (gy + 1) * buffer.height / by;
                for (int y = y0; y < y1; y++) {
                    for (int x = x0; x < x1; x++) {
                        if (x < buffer.width && y < buffer.height) {
                            const uint8_t* src = tmp.pixelAt(x, y);
                            uint8_t* dst = buffer.pixelAt(x, y);
                            dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3];
                        }
                    }
                }
            }
        }
    }
}

} // namespace FreeEffect
