#include "../effect_registry.h"
#include "solid_composite_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<SolidCompositeEffect> s_reg("Solid Composite", "Channel");

SolidCompositeEffect::SolidCompositeEffect() {
    addParameter(EffectParameter::makeColor("color", "Color", {0.0, 0.0, 0.0, 1.0}));
    addParameter(EffectParameter::makeFloat("opacity", "Opacity", 0.0, 100.0, 100.0));
    addParameter(EffectParameter::makeDropdown("compositeMode", "Composite Mode",
        {"Normal", "Add", "Multiply", "Screen", "Overlay"}, 0));
}

std::unique_ptr<Effect> SolidCompositeEffect::clone() const {
    auto e = std::make_unique<SolidCompositeEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void SolidCompositeEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    Color solid = getColorParam("color");
    float opacity = getFloatParam("opacity") / 100.0f;
    int mode = getDropdownParam("compositeMode");

    float bgR = solid.r, bgG = solid.g, bgB = solid.b, bgA = solid.a;

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            float fgR = p[0] / 255.0f, fgG = p[1] / 255.0f, fgB = p[2] / 255.0f;
            float fgA = p[3] / 255.0f;
            float resultR = 0, resultG = 0, resultB = 0;

            auto blendNormal = [](float f, float b) { return f; };
            auto blendAdd = [](float f, float b) { return f + b; };
            auto blendMultiply = [](float f, float b) { return f * b; };
            auto blendScreen = [](float f, float b) { return 1.0f - (1.0f - f) * (1.0f - b); };
            auto blendOverlay = [](float f, float b) {
                return b < 0.5f ? 2.0f * f * b : 1.0f - 2.0f * (1.0f - f) * (1.0f - b);
            };

            switch (mode) {
                case 0: resultR = blendNormal(fgR, bgR); resultG = blendNormal(fgG, bgG); resultB = blendNormal(fgB, bgB); break;
                case 1: resultR = blendAdd(fgR, bgR); resultG = blendAdd(fgG, bgG); resultB = blendAdd(fgB, bgB); break;
                case 2: resultR = blendMultiply(fgR, bgR); resultG = blendMultiply(fgG, bgG); resultB = blendMultiply(fgB, bgB); break;
                case 3: resultR = blendScreen(fgR, bgR); resultG = blendScreen(fgG, bgG); resultB = blendScreen(fgB, bgB); break;
                case 4: resultR = blendOverlay(fgR, bgR); resultG = blendOverlay(fgG, bgG); resultB = blendOverlay(fgB, bgB); break;
            }

            p[0] = static_cast<uint8_t>(std::clamp((fgR * (1.0f - opacity) + resultR * opacity) * 255.0f, 0.0f, 255.0f));
            p[1] = static_cast<uint8_t>(std::clamp((fgG * (1.0f - opacity) + resultG * opacity) * 255.0f, 0.0f, 255.0f));
            p[2] = static_cast<uint8_t>(std::clamp((fgB * (1.0f - opacity) + resultB * opacity) * 255.0f, 0.0f, 255.0f));
        }
    }
}

} // namespace FreeEffect
