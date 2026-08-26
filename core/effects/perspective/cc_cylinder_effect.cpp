#include "../effect_registry.h"
#include "cc_cylinder_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<CCCylinderEffect> s_reg("CC Cylinder", "Perspective");

CCCylinderEffect::CCCylinderEffect() {
    addParameter(EffectParameter::makeFloat("radius", "Radius", 10.0, 500.0, 200.0));
    addParameter(EffectParameter::makeVec2("position", "Position", {50.0, 50.0}));
    addParameter(EffectParameter::makeAngle("rotation", "Rotation", 0.0));
    addParameter(EffectParameter::makeFloat("lightIntensity", "Light Intensity", 0.0, 10.0, 1.0));
}

std::vector<ParameterGroup> CCCylinderEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeFloat("radius", "Radius", 10.0, 500.0, false),
        EffectParameter::makeVec2("position", "Position", {50.0, 50.0}),
        EffectParameter::makeAngle("rotation", "Rotation", 0.0),
        EffectParameter::makeFloat("lightIntensity", "Light Intensity", 0.0, 10.0, false)
    }}};
}

std::unique_ptr<Effect> CCCylinderEffect::clone() const {
    auto e = std::make_unique<CCCylinderEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void CCCylinderEffect::render(PixelBuffer& buffer, double time) {
    float radius = getFloatParam("radius");
    float rot = getAngleParam("rotation") * 3.14159265f / 180.0f;
    float lightIntensity = getFloatParam("lightIntensity");

    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);
    std::fill(tmp.data.begin(), tmp.data.end(), 0);

    float cx = buffer.width / 2.0f;
    float cy = buffer.height / 2.0f;

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            float dx = x - cx;
            float normX = dx / std::max(radius, 1.0f);
            if (std::abs(normX) > 1.0f) continue;

            float angle = std::asin(normX) + rot;
            float depth = std::cos(angle);
            if (depth < 0) continue;

            int srcX = static_cast<int>((angle + 3.14159265f) / (2.0f * 3.14159265f) * buffer.width);
            srcX = std::clamp(srcX, 0, buffer.width - 1);
            int srcY = y;
            if (srcY >= 0 && srcY < buffer.height) {
                const uint8_t* src = buffer.pixelAt(srcX, srcY);
                float lighting = depth * lightIntensity;
                uint8_t* dst = tmp.pixelAt(x, y);
                dst[0] = static_cast<uint8_t>(std::clamp(static_cast<double>(src[0] * lighting), 0.0, 255.0));
                dst[1] = static_cast<uint8_t>(std::clamp(static_cast<double>(src[1] * lighting), 0.0, 255.0));
                dst[2] = static_cast<uint8_t>(std::clamp(static_cast<double>(src[2] * lighting), 0.0, 255.0));
                dst[3] = static_cast<uint8_t>(depth * 255.0f);
            }
        }
    }
    buffer.data = tmp.data;
}

} // namespace FreeEffect
