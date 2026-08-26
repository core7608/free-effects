#include "../effect_registry.h"
#include "bevel_alpha_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<BevelAlphaEffect> s_reg("Bevel Alpha", "Stylize");

BevelAlphaEffect::BevelAlphaEffect() {
    addParameter(EffectParameter::makeFloat("edgeThickness", "Edge Thickness", 1.0, 50.0, 5.0));
    addParameter(EffectParameter::makeAngle("lightAngle", "Light Angle", -60.0));
    addParameter(EffectParameter::makeColor("lightColor", "Light Color", {255.0, 255.0, 255.0, 1.0}));
    addParameter(EffectParameter::makeFloat("lightIntensity", "Light Intensity", 0.0, 10.0, 1.0));
}

std::vector<ParameterGroup> BevelAlphaEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeFloat("edgeThickness", "Edge Thickness", 1.0, 50.0, false),
        EffectParameter::makeAngle("lightAngle", "Light Angle", -60.0),
        EffectParameter::makeColor("lightColor", "Light Color", {255.0, 255.0, 255.0, 1.0}),
        EffectParameter::makeFloat("lightIntensity", "Light Intensity", 0.0, 10.0, false)
    }}};
}

std::unique_ptr<Effect> BevelAlphaEffect::clone() const {
    auto e = std::make_unique<BevelAlphaEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void BevelAlphaEffect::render(PixelBuffer& buffer, double time) {
    float thickness = getFloatParam("edgeThickness");
    float lightAngle = getAngleParam("lightAngle") * 3.14159265f / 180.0f;
    Color lc = getColorParam("lightColor");
    float intensity = getFloatParam("lightIntensity");
    int radius = static_cast<int>(thickness);
    float lx = std::cos(lightAngle);
    float ly = std::sin(lightAngle);

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            float alpha = p[3] / 255.0f;
            if (alpha < 0.01f) continue;

            float nx = 0, ny = 0;
            for (int kx = -1; kx <= 1; kx++) {
                for (int ky = -1; ky <= 1; ky++) {
                    int sx = std::clamp(x + kx, 0, buffer.width - 1);
                    int sy = std::clamp(y + ky, 0, buffer.height - 1);
                    float sa = buffer.pixelAt(sx, sy)[3] / 255.0f;
                    nx += (sa - alpha) * kx;
                    ny += (sa - alpha) * ky;
                }
            }

            float dot = nx * lx + ny * ly;
            float highlight = std::max(dot, 0.0f) * intensity;
            float shadow = std::max(-dot, 0.0f) * intensity * 0.5f;

            float r = p[0] + lc.r * highlight - 128.0f * shadow;
            float g = p[1] + lc.g * highlight - 128.0f * shadow;
            float b = p[2] + lc.b * highlight - 128.0f * shadow;
            p[0] = static_cast<uint8_t>(std::clamp(static_cast<double>(r), 0.0, 255.0));
            p[1] = static_cast<uint8_t>(std::clamp(static_cast<double>(g), 0.0, 255.0));
            p[2] = static_cast<uint8_t>(std::clamp(static_cast<double>(b), 0.0, 255.0));
        }
    }
}

} // namespace FreeEffect
