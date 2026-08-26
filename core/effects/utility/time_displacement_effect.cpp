#include "../effect_registry.h"
#include "time_displacement_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<TimeDisplacementEffect> s_reg("Time Displacement", "Utility");

TimeDisplacementEffect::TimeDisplacementEffect() {
    addParameter(EffectParameter::makeString("displacementLayer", "Displacement Layer", ""));
    addParameter(EffectParameter::makeFloat("maxDisplacement", "Max Displacement Time", -10.0, 10.0, 1.0));
    addParameter(EffectParameter::makeBool("luminance", "Use For Luminance", true));
}

std::vector<ParameterGroup> TimeDisplacementEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeString("displacementLayer", "Displacement Layer", ""),
        EffectParameter::makeFloat("maxDisplacement", "Max Displacement Time", -10.0, 10.0, false),
        EffectParameter::makeBool("luminance", "Use For Luminance", true)
    }}};
}

std::unique_ptr<Effect> TimeDisplacementEffect::clone() const {
    auto e = std::make_unique<TimeDisplacementEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void TimeDisplacementEffect::render(PixelBuffer& buffer, double time) {
    float maxDisp = getFloatParam("maxDisplacement");

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            float lum = (p[0] * 0.299f + p[1] * 0.587f + p[2] * 0.114f) / 255.0f;
            float displacement = (lum - 0.5f) * 2.0f * maxDisp;

            int srcX = x;
            int srcY = y;
            if (srcX >= 0 && srcX < buffer.width && srcY >= 0 && srcY < buffer.height) {
                uint8_t* dst = buffer.pixelAt(x, y);
                const uint8_t* src = buffer.pixelAt(
                    std::clamp(srcX, 0, buffer.width - 1),
                    std::clamp(srcY, 0, buffer.height - 1));
                float fade = 1.0f - std::abs(displacement) * 0.1f;
                fade = std::clamp(fade, 0.0f, 1.0f);
                dst[0] = static_cast<uint8_t>(src[0] * fade);
                dst[1] = static_cast<uint8_t>(src[1] * fade);
                dst[2] = static_cast<uint8_t>(src[2] * fade);
            }
        }
    }
}

} // namespace FreeEffect
