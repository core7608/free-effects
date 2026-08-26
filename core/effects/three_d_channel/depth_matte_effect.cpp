#include "../effect_registry.h"
#include "depth_matte_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<DepthMatteEffect> s_reg("Depth Matte", "3D Channel");

DepthMatteEffect::DepthMatteEffect() {
    addParameter(EffectParameter::makeFloat("depth", "Depth", 0.0, 10000.0, 5000.0));
    addParameter(EffectParameter::makeFloat("feather", "Feather", 0.0, 1000.0, 0.0));
    addParameter(EffectParameter::makeDropdown("edgeType", "Edge", {"Feather", "None", "Invert"}, 0));
}

std::unique_ptr<Effect> DepthMatteEffect::clone() const {
    auto e = std::make_unique<DepthMatteEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void DepthMatteEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    float depth = getFloatParam("depth");
    float feather = getFloatParam("feather");
    int edge = getDropdownParam("edgeType");

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            float simDepth = static_cast<float>(y) / buffer.height * 10000.0f;
            float alpha = std::clamp((simDepth - depth + feather) / (feather * 2.0f + 0.001f), 0.0f, 1.0f);
            if (edge == 1) alpha = (simDepth > depth) ? 1.0f : 0.0f;
            if (edge == 2) alpha = 1.0f - alpha;
            p[3] = static_cast<uint8_t>(alpha * 255.0f);
        }
    }
}

} // namespace FreeEffect
