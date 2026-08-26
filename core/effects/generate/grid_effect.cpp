#include "../effect_registry.h"
#include "grid_effect.h"
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<GridEffect> s_reg("Grid", "Generate");

GridEffect::GridEffect() {
    addParameter(EffectParameter::makeVec2("anchor", "Anchor", {0.0, 0.0}));
    addParameter(EffectParameter::makeVec2("size", "Size", {50.0, 50.0}));
    addParameter(EffectParameter::makeFloat("border", "Border Width", 1.0, 50.0, 2.0));
    addParameter(EffectParameter::makeColor("color", "Color", {255.0, 255.0, 255.0, 1.0}));
}

std::vector<ParameterGroup> GridEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeVec2("anchor", "Anchor", {0.0, 0.0}),
        EffectParameter::makeVec2("size", "Size", {50.0, 50.0}),
        EffectParameter::makeFloat("border", "Border Width", 1.0, 50.0, false),
        EffectParameter::makeColor("color", "Color", {255.0, 255.0, 255.0, 1.0})
    }}};
}

std::unique_ptr<Effect> GridEffect::clone() const {
    auto e = std::make_unique<GridEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void GridEffect::render(PixelBuffer& buffer, double time) {
    Vec2 anchor = getVec2Param("anchor");
    Vec2 size = getVec2Param("size");
    float border = getFloatParam("border");
    Color c = getColorParam("color");

    float ax = anchor.x;
    float ay = anchor.y;
    float sw = std::max(size.x, 1.0);
    float sh = std::max(size.y, 1.0);
    float halfBorder = border / 2.0f;

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            float fx = std::fmod(x - ax + sw * 1000.0f, sw);
            float fy = std::fmod(y - ay + sh * 1000.0f, sh);
            bool onLine = (fx < border || fy < border);
            if (onLine) {
                uint8_t* p = buffer.pixelAt(x, y);
                float srcA = p[3] / 255.0f;
                float outA = c.a + srcA * (1.0f - c.a);
                if (outA > 0) {
                    p[0] = static_cast<uint8_t>((c.r * c.a + p[0] * srcA * (1.0f - c.a)) / outA);
                    p[1] = static_cast<uint8_t>((c.g * c.a + p[1] * srcA * (1.0f - c.a)) / outA);
                    p[2] = static_cast<uint8_t>((c.b * c.a + p[2] * srcA * (1.0f - c.a)) / outA);
                }
                p[3] = static_cast<uint8_t>(outA * 255.0f);
            }
        }
    }
}

} // namespace FreeEffect
