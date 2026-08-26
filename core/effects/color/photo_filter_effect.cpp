#include "../effect_registry.h"
#include "photo_filter_effect.h"
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<PhotoFilterEffect> s_reg("Photo Filter", "Color Correction");

PhotoFilterEffect::PhotoFilterEffect() {
    addParameter(EffectParameter::makeColor("filterColor", "Filter Color", {255.0, 140.0, 0.0, 1.0}));
    addParameter(EffectParameter::makeFloat("density", "Density", 0.0, 100.0, 50.0));
}

std::vector<ParameterGroup> PhotoFilterEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeColor("filterColor", "Filter Color", {255.0, 140.0, 0.0, 1.0}),
        EffectParameter::makeFloat("density", "Density", 0.0, 100.0, false)
    }}};
}

std::unique_ptr<Effect> PhotoFilterEffect::clone() const {
    auto e = std::make_unique<PhotoFilterEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void PhotoFilterEffect::render(PixelBuffer& buffer, double time) {
    Color fc = getColorParam("filterColor");
    float density = getFloatParam("density") / 100.0f;

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            float r = p[0] + (fc.r - p[0]) * density;
            float g = p[1] + (fc.g - p[1]) * density;
            float b = p[2] + (fc.b - p[2]) * density;
            p[0] = static_cast<uint8_t>(std::clamp(static_cast<double>(r), 0.0, 255.0));
            p[1] = static_cast<uint8_t>(std::clamp(static_cast<double>(g), 0.0, 255.0));
            p[2] = static_cast<uint8_t>(std::clamp(static_cast<double>(b), 0.0, 255.0));
        }
    }
}

} // namespace FreeEffect
