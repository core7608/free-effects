#include "../effect_registry.h"
#include "cc_ball_action_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<CCBallActionEffect> s_reg("CC Ball Action", "Simulation");

CCBallActionEffect::CCBallActionEffect() {
    addParameter(EffectParameter::makeFloat("gridSpacing", "Grid Spacing", 1.0, 50.0, 5.0));
    addParameter(EffectParameter::makeFloat("ballSize", "Ball Size", 0.0, 100.0, 100.0));
    addParameter(EffectParameter::makeAngle("scatter", "Scatter", 0.0));
    addParameter(EffectParameter::makeFloat("gravity", "Gravity", 0.0, 500.0, 0.0));
}

std::unique_ptr<Effect> CCBallActionEffect::clone() const {
    auto e = std::make_unique<CCBallActionEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void CCBallActionEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    float spacing = getFloatParam("gridSpacing");
    float ballSize = getFloatParam("ballSize") / 100.0f;
    float gravity = getFloatParam("gravity");

    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* dst = tmp.pixelAt(x, y);
            dst[0] = dst[1] = dst[2] = 0; dst[3] = 0;
        }
    }

    for (int gy = 0; gy < buffer.height; gy += static_cast<int>(spacing)) {
        for (int gx = 0; gx < buffer.width; gx += static_cast<int>(spacing)) {
            int sx = std::min(gx + static_cast<int>(spacing / 2), buffer.width - 1);
            int sy = std::min(gy + static_cast<int>(spacing / 2), buffer.height - 1);
            const uint8_t* src = buffer.pixelAt(sx, sy);
            float ballR = spacing * ballSize * 0.5f;

            for (int dy = static_cast<int>(-ballR); dy <= static_cast<int>(ballR); dy++) {
                for (int dx = static_cast<int>(-ballR); dx <= static_cast<int>(ballR); dx++) {
                    if (dx * dx + dy * dy > ballR * ballR) continue;
                    int px = gx + static_cast<int>(spacing / 2) + dx;
                    int py = gy + static_cast<int>(spacing / 2) + dy;
                    if (px >= 0 && px < buffer.width && py >= 0 && py < buffer.height) {
                        uint8_t* dst = tmp.pixelAt(px, py);
                        float shade = 1.0f - std::sqrt(static_cast<float>(dx*dx+dy*dy)) / ballR;
                        dst[0] = static_cast<uint8_t>(src[0] * shade);
                        dst[1] = static_cast<uint8_t>(src[1] * shade);
                        dst[2] = static_cast<uint8_t>(src[2] * shade);
                        dst[3] = static_cast<uint8_t>(shade * 255.0f);
                    }
                }
            }
        }
    }
    buffer.data = tmp.data;
}

} // namespace FreeEffect
