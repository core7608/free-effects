#include "../effect_registry.h"
#include "displacement_map_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<DisplacementMapEffect> s_reg("Displacement Map", "Distort");

DisplacementMapEffect::DisplacementMapEffect() {
    addParameter(EffectParameter::makeFloat("maxHorizontalDisplacement", "Max Horizontal Displacement", -1000.0, 1000.0, 0.0));
    addParameter(EffectParameter::makeFloat("maxVerticalDisplacement", "Max Vertical Displacement", -1000.0, 1000.0, 0.0));
    addParameter(EffectParameter::makeDropdown("redSource", "Red Displacement", {"Red", "Green", "Blue", "Alpha", "Luminance", "Hue", "Saturation", "Lightness"}, 0));
    addParameter(EffectParameter::makeDropdown("greenSource", "Green Displacement", {"Red", "Green", "Blue", "Alpha", "Luminance", "Hue", "Saturation", "Lightness"}, 1));
    addParameter(EffectParameter::makeDropdown("behavior", "Displacement Behavior", {"Center Map", "Stretch Map", "Tile Map"}, 0));
}

std::unique_ptr<Effect> DisplacementMapEffect::clone() const {
    auto e = std::make_unique<DisplacementMapEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void DisplacementMapEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    float maxH = getFloatParam("maxHorizontalDisplacement");
    float maxV = getFloatParam("maxVerticalDisplacement");
    int redSrc = getDropdownParam("redSource");
    int greenSrc = getDropdownParam("greenSource");

    auto getChannelVal = [](const uint8_t* p, int ch) -> float {
        if (ch == 0) return p[0] / 255.0f;
        if (ch == 1) return p[1] / 255.0f;
        if (ch == 2) return p[2] / 255.0f;
        if (ch == 3) return p[3] / 255.0f;
        return (0.299f * p[0] + 0.587f * p[1] + 0.114f * p[2]) / 255.0f;
    };

    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);
    std::copy(buffer.data.begin(), buffer.data.end(), tmp.data.begin());

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            const uint8_t* map = tmp.pixelAt(x, y);
            float dispX = (getChannelVal(map, redSrc) - 0.5f) * 2.0f * maxH;
            float dispY = (getChannelVal(map, greenSrc) - 0.5f) * 2.0f * maxV;

            int sx = std::clamp(static_cast<int>(x + dispX), 0, buffer.width - 1);
            int sy = std::clamp(static_cast<int>(y + dispY), 0, buffer.height - 1);
            const uint8_t* src = tmp.pixelAt(sx, sy);
            uint8_t* dst = buffer.pixelAt(x, y);
            dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3];
        }
    }
}

} // namespace FreeEffect
