#include "../effect_registry.h"
#include "perspective_drop_shadow_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<PerspectiveDropShadowEffect> s_reg("Drop Shadow", "Perspective");

PerspectiveDropShadowEffect::PerspectiveDropShadowEffect() {
    addParameter(EffectParameter::makeColor("color", "Shadow Color", {0.0, 0.0, 0.0, 0.75}));
    addParameter(EffectParameter::makeAngle("direction", "Direction", 135.0));
    addParameter(EffectParameter::makeFloat("distance", "Distance", 0.0, 200.0, 5.0));
    addParameter(EffectParameter::makeFloat("softness", "Softness", 0.0, 100.0, 5.0));
}

std::vector<ParameterGroup> PerspectiveDropShadowEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeColor("color", "Shadow Color", {0.0, 0.0, 0.0, 0.75}),
        EffectParameter::makeAngle("direction", "Direction", 135.0),
        EffectParameter::makeFloat("distance", "Distance", 0.0, 200.0, false),
        EffectParameter::makeFloat("softness", "Softness", 0.0, 100.0, false)
    }}};
}

std::unique_ptr<Effect> PerspectiveDropShadowEffect::clone() const {
    auto e = std::make_unique<PerspectiveDropShadowEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void PerspectiveDropShadowEffect::render(PixelBuffer& buffer, double time) {
    Color sc = getColorParam("color");
    float dir = getAngleParam("direction") * 3.14159265f / 180.0f;
    float dist = getFloatParam("distance");
    float softness = getFloatParam("softness");
    int radius = static_cast<int>(softness);
    float offsetX = std::cos(dir) * dist;
    float offsetY = std::sin(dir) * dist;

    PixelBuffer shadow;
    shadow.resize(buffer.width, buffer.height);

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            const uint8_t* src = buffer.pixelAt(x, y);
            float alpha = src[3] / 255.0f;
            if (alpha > 0.01f) {
                int sx = static_cast<int>(x + offsetX);
                int sy = static_cast<int>(y + offsetY);
                if (radius > 0) {
                    for (int ky = -radius; ky <= radius; ky++) {
                        for (int kx = -radius; kx <= radius; kx++) {
                            int px = std::clamp(sx + kx, 0, buffer.width - 1);
                            int py = std::clamp(sy + ky, 0, buffer.height - 1);
                            float d = std::sqrt(static_cast<float>(kx * kx + ky * ky));
                            if (d <= radius) {
                                float w = 1.0f - d / radius;
                                w *= w;
                                uint8_t* dst = shadow.pixelAt(px, py);
                                float add = alpha * sc.a * w;
                                float existing = dst[3] / 255.0f;
                                float out = std::min(existing + add * (1.0f - existing), 1.0f);
                                dst[0] = static_cast<uint8_t>(sc.r * out);
                                dst[1] = static_cast<uint8_t>(sc.g * out);
                                dst[2] = static_cast<uint8_t>(sc.b * out);
                                dst[3] = static_cast<uint8_t>(out * 255.0f);
                            }
                        }
                    }
                } else {
                    int px = std::clamp(sx, 0, buffer.width - 1);
                    int py = std::clamp(sy, 0, buffer.height - 1);
                    uint8_t* dst = shadow.pixelAt(px, py);
                    float add = alpha * sc.a;
                    float existing = dst[3] / 255.0f;
                    float out = std::min(existing + add * (1.0f - existing), 1.0f);
                    dst[0] = static_cast<uint8_t>(sc.r * out);
                    dst[1] = static_cast<uint8_t>(sc.g * out);
                    dst[2] = static_cast<uint8_t>(sc.b * out);
                    dst[3] = static_cast<uint8_t>(out * 255.0f);
                }
            }
        }
    }

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            const uint8_t* s = shadow.pixelAt(x, y);
            float sa = s[3] / 255.0f;
            float pa = p[3] / 255.0f;
            float outA = sa + pa * (1.0f - sa);
            if (outA > 0) {
                p[0] = static_cast<uint8_t>((s[0] * sa + p[0] * pa * (1.0f - sa)) / outA);
                p[1] = static_cast<uint8_t>((s[1] * sa + p[1] * pa * (1.0f - sa)) / outA);
                p[2] = static_cast<uint8_t>((s[2] * sa + p[2] * pa * (1.0f - sa)) / outA);
            }
            p[3] = static_cast<uint8_t>(outA * 255.0f);
        }
    }
}

} // namespace FreeEffect
