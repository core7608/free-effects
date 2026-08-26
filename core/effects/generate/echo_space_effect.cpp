#include "../effect_registry.h"
#include "echo_space_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<EchoSpaceEffect> s_reg("Echo Space", "Generate");

EchoSpaceEffect::EchoSpaceEffect() {
    addParameter(EffectParameter::makeInt("numEchoes", "Number of Echoes", 0, 20, 3));
    addParameter(EffectParameter::makeFloat("startTime", "Starting Intensity", 0.0, 100.0, 100.0));
    addParameter(EffectParameter::makeFloat("decay", "Decay", 0.0, 100.0, 50.0));
    addParameter(EffectParameter::makeVec2("offset", "Offset", {10.0, 10.0}));
}

std::unique_ptr<Effect> EchoSpaceEffect::clone() const {
    auto e = std::make_unique<EchoSpaceEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void EchoSpaceEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    int echoes = getIntParam("numEchoes");
    float startInt = getFloatParam("startTime") / 100.0f;
    float decay = getFloatParam("decay") / 100.0f;
    Vec2 offset = getVec2Param("offset");

    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);
    std::copy(buffer.data.begin(), buffer.data.end(), tmp.data.begin());

    for (int echo = 1; echo <= echoes; echo++) {
        float alpha = startInt * std::pow(1.0f - decay, static_cast<float>(echo));
        if (alpha < 0.01f) continue;
        float offX = offset.x * echo, offY = offset.y * echo;
        for (int y = 0; y < buffer.height; y++) {
            for (int x = 0; x < buffer.width; x++) {
                int sx = std::clamp(static_cast<int>(x - offX), 0, buffer.width - 1);
                int sy = std::clamp(static_cast<int>(y - offY), 0, buffer.height - 1);
                const uint8_t* p = tmp.pixelAt(sx, sy);
                uint8_t* dst = buffer.pixelAt(x, y);
                dst[0] = static_cast<uint8_t>(std::min(static_cast<double>(dst[0] + p[0] * alpha), 255.0));
                dst[1] = static_cast<uint8_t>(std::min(static_cast<double>(dst[1] + p[1] * alpha), 255.0));
                dst[2] = static_cast<uint8_t>(std::min(static_cast<double>(dst[2] + p[2] * alpha), 255.0));
            }
        }
    }
}

} // namespace FreeEffect
