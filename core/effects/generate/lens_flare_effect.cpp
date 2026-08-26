#include "../effect_registry.h"
#include "lens_flare_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<LensFlareEffect> s_reg("Lens Flare", "Generate");

LensFlareEffect::LensFlareEffect() {
    addParameter(EffectParameter::makeVec2("center", "Center", {0.5, 0.5}));
    addParameter(EffectParameter::makeFloat("flareBrightness", "Flare Brightness", 0.0, 100.0, 50.0));
    addParameter(EffectParameter::makeInt("lensType", "Lens Type", 1, 5, 3));
}

std::vector<ParameterGroup> LensFlareEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeVec2("center", "Center", {0.5, 0.5}),
        EffectParameter::makeFloat("flareBrightness", "Flare Brightness", 0.0, 100.0, false),
        EffectParameter::makeInt("lensType", "Lens Type", 1, 5, false)
    }}};
}

std::unique_ptr<Effect> LensFlareEffect::clone() const {
    auto e = std::make_unique<LensFlareEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void LensFlareEffect::render(PixelBuffer& buffer, double time) {
    Vec2 center = getVec2Param("center");
    float brightness = getFloatParam("flareBrightness") / 100.0f;
    float cx = center.x * buffer.width;
    float cy = center.y * buffer.height;
    float maxDist = std::sqrt(cx * cx + cy * cy);

    struct FlareRing {
        float dx, dy, radius, r, g, b, intensity;
    };
    std::vector<FlareRing> rings = {
        {0.0f, 0.0f, 30.0f * brightness, 255, 255, 200, 1.0f},
        {0.2f, 0.2f, 20.0f * brightness, 200, 220, 255, 0.6f},
        {-0.15f, -0.15f, 40.0f * brightness, 255, 180, 100, 0.4f},
        {0.3f, 0.3f, 15.0f * brightness, 180, 255, 180, 0.3f},
    };

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            float r = 0, g = 0, b = 0, a = 0;
            for (const auto& ring : rings) {
                float fx = cx + ring.dx * buffer.width;
                float fy = cy + ring.dy * buffer.height;
                float dx = x - fx;
                float dy = y - fy;
                float dist = std::sqrt(dx * dx + dy * dy);
                if (dist < ring.radius) {
                    float falloff = 1.0f - dist / ring.radius;
                    falloff = falloff * falloff;
                    r += ring.r * falloff * ring.intensity;
                    g += ring.g * falloff * ring.intensity;
                    b += ring.b * falloff * ring.intensity;
                    a = std::max(a, falloff * ring.intensity);
                }
            }
            if (a > 0.01f) {
                uint8_t* p = buffer.pixelAt(x, y);
                float srcA = p[3] / 255.0f;
                float outA = std::min(a, 1.0f) + srcA * (1.0f - std::min(a, 1.0f));
                if (outA > 0) {
                    p[0] = static_cast<uint8_t>(std::clamp(
                        (r * brightness + p[0] * srcA * (1.0f - a)) / outA, 0.0f, 255.0f));
                    p[1] = static_cast<uint8_t>(std::clamp(
                        (g * brightness + p[1] * srcA * (1.0f - a)) / outA, 0.0f, 255.0f));
                    p[2] = static_cast<uint8_t>(std::clamp(
                        (b * brightness + p[2] * srcA * (1.0f - a)) / outA, 0.0f, 255.0f));
                }
                p[3] = static_cast<uint8_t>(outA * 255.0f);
            }
        }
    }
}

} // namespace FreeEffect
