#include "../effect_registry.h"
#include "cc_pixel_polly_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<CCPixelPollyEffect> s_reg("CC Pixel Polly", "Simulation");

CCPixelPollyEffect::CCPixelPollyEffect() {
    addParameter(EffectParameter::makeFloat("force", "Force", 0.0, 1000.0, 100.0));
    addParameter(EffectParameter::makeFloat("gravity", "Gravity", 0.0, 500.0, 200.0));
    addParameter(EffectParameter::makeInt("pieceSize", "Grid Spacing", 1, 50, 5));
}

std::unique_ptr<Effect> CCPixelPollyEffect::clone() const {
    auto e = std::make_unique<CCPixelPollyEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void CCPixelPollyEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    float force = getFloatParam("force");
    float grav = getFloatParam("gravity");
    int pieceSize = getIntParam("pieceSize");

    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);
    std::copy(buffer.data.begin(), buffer.data.end(), tmp.data.begin());

    float progress = std::clamp(static_cast<float>(time) * 0.5f, 0.0f, 1.0f);
    int threshold = static_cast<int>(progress * buffer.height);

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* dst = buffer.pixelAt(x, y);
            if (y < threshold) {
                float dx = (x - buffer.width * 0.5f) * force * 0.001f;
                float dyOff = force * progress * 0.1f;
                int sx = std::clamp(static_cast<int>(x + dx * progress), 0, buffer.width - 1);
                int sy = std::clamp(y + static_cast<int>(dyOff), 0, buffer.height - 1);
                const uint8_t* src = tmp.pixelAt(sx, sy);
                dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = 0;
            }
        }
    }
}

} // namespace FreeEffect
