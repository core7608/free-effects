#include "../effect_registry.h"
#include "checkerboard_effect.h"
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<CheckerboardEffect> s_reg("Checkerboard", "Generate");

CheckerboardEffect::CheckerboardEffect() {
    addParameter(EffectParameter::makeFloat("size", "Size", 1.0, 500.0, 50.0));
    addParameter(EffectParameter::makeVec2("anchor", "Anchor", {0.0, 0.0}));
    addParameter(EffectParameter::makeColor("color1", "Color 1", {255.0, 255.0, 255.0, 1.0}));
    addParameter(EffectParameter::makeColor("color2", "Color 2", {0.0, 0.0, 0.0, 1.0}));
}

std::vector<ParameterGroup> CheckerboardEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeFloat("size", "Size", 1.0, 500.0, false),
        EffectParameter::makeVec2("anchor", "Anchor", {0.0, 0.0}),
        EffectParameter::makeColor("color1", "Color 1", {255.0, 255.0, 255.0, 1.0}),
        EffectParameter::makeColor("color2", "Color 2", {0.0, 0.0, 0.0, 1.0})
    }}};
}

std::unique_ptr<Effect> CheckerboardEffect::clone() const {
    auto e = std::make_unique<CheckerboardEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void CheckerboardEffect::render(PixelBuffer& buffer, double time) {
    float size = getFloatParam("size");
    Vec2 anchor = getVec2Param("anchor");
    Color c1 = getColorParam("color1");
    Color c2 = getColorParam("color2");
    float s = std::max(size, 1.0f);

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            int cx = static_cast<int>(std::floor((x - anchor.x) / s));
            int cy = static_cast<int>(std::floor((y - anchor.y) / s));
            bool odd = ((cx + cy) & 1) != 0;
            Color c = odd ? c2 : c1;
            uint8_t* p = buffer.pixelAt(x, y);
            p[0] = static_cast<uint8_t>(std::clamp(c.r, 0.0, 255.0));
            p[1] = static_cast<uint8_t>(std::clamp(c.g, 0.0, 255.0));
            p[2] = static_cast<uint8_t>(std::clamp(c.b, 0.0, 255.0));
            p[3] = static_cast<uint8_t>(std::clamp(c.a * 255.0, 0.0, 255.0));
        }
    }
}

} // namespace FreeEffect
