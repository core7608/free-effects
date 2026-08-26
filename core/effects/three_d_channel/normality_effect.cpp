#include "../effect_registry.h"
#include "normality_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<NormalityEffect> s_reg("Normality", "3D Channel");

NormalityEffect::NormalityEffect() {
    addParameter(EffectParameter::makeFloat("falloff", "Falloff", 0.0, 100.0, 50.0));
    addParameter(EffectParameter::makeAngle("lightAngle", "Light Angle", -45.0));
}

std::unique_ptr<Effect> NormalityEffect::clone() const {
    auto e = std::make_unique<NormalityEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void NormalityEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    float falloff = getFloatParam("falloff") / 100.0f;
    float lightAngle = getFloatParam("lightAngle") * 3.14159265f / 180.0f;
    float lx = std::cos(lightAngle), ly = std::sin(lightAngle);

    for (int y = 1; y < buffer.height - 1; y++) {
        for (int x = 1; x < buffer.width - 1; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            const uint8_t* l = buffer.pixelAt(x-1, y);
            const uint8_t* r2 = buffer.pixelAt(x+1, y);
            const uint8_t* t = buffer.pixelAt(x, y-1);
            const uint8_t* b2 = buffer.pixelAt(x, y+1);
            float dX = (r2[0] - l[0]) / 255.0f;
            float dY = (b2[0] - t[0]) / 255.0f;
            float nz = 1.0f - falloff * (std::abs(dX) + std::abs(dY));
            float shade = std::max(0.0f, dX * lx + dY * ly + nz * std::sqrt(1.0f - lx*lx - ly*ly));
            p[0] = static_cast<uint8_t>(std::clamp(dX * 128.0f + 128.0f, 0.0f, 255.0f));
            p[1] = static_cast<uint8_t>(std::clamp(dY * 128.0f + 128.0f, 0.0f, 255.0f));
            p[2] = static_cast<uint8_t>(std::clamp(nz * 255.0f, 0.0f, 255.0f));
        }
    }
}

} // namespace FreeEffect
