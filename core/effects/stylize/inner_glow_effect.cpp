#include "../effect_registry.h"
#include "inner_glow_effect.h"
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<InnerGlowEffect> s_reg("Inner Glow", "Stylize");

InnerGlowEffect::InnerGlowEffect() {
    addParameter(EffectParameter::makeColor("color", "Glow Color", {255.0, 255.0, 255.0, 1.0}));
    addParameter(EffectParameter::makeFloat("opacity", "Opacity", 0.0, 100.0, 75.0));
    addParameter(EffectParameter::makeFloat("radius", "Radius", 0.0, 200.0, 10.0));
    addParameter(EffectParameter::makeFloat("choke", "Choke", 0.0, 100.0, 0.0));
    addParameter(EffectParameter::makeDropdown("source", "Source", {"Edge", "Center"}, 0));
}

std::vector<ParameterGroup> InnerGlowEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeColor("color", "Glow Color", {255.0, 255.0, 255.0, 1.0}),
        EffectParameter::makeFloat("opacity", "Opacity", 0.0, 100.0, false),
        EffectParameter::makeFloat("radius", "Radius", 0.0, 200.0, false),
        EffectParameter::makeFloat("choke", "Choke", 0.0, 100.0, false),
        EffectParameter::makeDropdown("source", "Source", {"Edge", "Center"}, 0)
    }}};
}

std::unique_ptr<Effect> InnerGlowEffect::clone() const {
    auto e = std::make_unique<InnerGlowEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void InnerGlowEffect::render(PixelBuffer& buffer, double time) {
    Color c = getColorParam("color");
    float opacity = getFloatParam("opacity") / 100.0f;
    int radius = static_cast<int>(getFloatParam("radius"));
    float choke = getFloatParam("choke") / 100.0f;
    int source = getDropdownParam("source");

    if (radius <= 0) return;
    float cx = buffer.width / 2.0f;
    float cy = buffer.height / 2.0f;

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            float alpha = p[3] / 255.0f;
            if (alpha < 0.01f) continue;

            float glowDist = 0.0f;
            if (source == 0) {
                int nearest = radius + 1;
                for (int ky = -radius; ky <= radius; ky++) {
                    for (int kx = -radius; kx <= radius; kx++) {
                        int nx = x + kx, ny = y + ky;
                        if (nx >= 0 && nx < buffer.width && ny >= 0 && ny < buffer.height) {
                            const uint8_t* np = buffer.pixelAt(nx, ny);
                            if (np[3] < 1) {
                                int d = std::abs(kx) + std::abs(ky);
                                if (d < nearest) nearest = d;
                            }
                        }
                    }
                }
                glowDist = static_cast<float>(nearest);
            } else {
                float dx = x - cx;
                float dy = y - cy;
                glowDist = std::sqrt(dx * dx + dy * dy);
            }

            float normDist = glowDist / radius;
            if (normDist < 1.0f && normDist >= choke) {
                float glow = 1.0f - (normDist - choke) / (1.0f - std::max(choke, 0.999f));
                glow = std::clamp(glow, 0.0f, 1.0f);
                glow *= opacity;
                float srcA = alpha;
                float outA = glow + srcA * (1.0f - glow);
                if (outA > 0) {
                    p[0] = static_cast<uint8_t>((c.r * glow + p[0] * srcA * (1.0f - glow)) / outA);
                    p[1] = static_cast<uint8_t>((c.g * glow + p[1] * srcA * (1.0f - glow)) / outA);
                    p[2] = static_cast<uint8_t>((c.b * glow + p[2] * srcA * (1.0f - glow)) / outA);
                }
                p[3] = static_cast<uint8_t>(outA * 255.0f);
            }
        }
    }
}

} // namespace FreeEffect
