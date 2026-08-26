#include "../effect_registry.h"
#include "leave_color_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<LeaveColorEffect> s_reg("Leave Color", "Color Correction");

LeaveColorEffect::LeaveColorEffect() {
    addParameter(EffectParameter::makeColor("colorToLeave", "Color To Leave", {255.0, 0.0, 0.0, 1.0}));
    addParameter(EffectParameter::makeFloat("tolerance", "Tolerance", 0.0, 100.0, 30.0));
    addParameter(EffectParameter::makeFloat("amountToDecolor", "Amount to Decolor", 0.0, 100.0, 100.0));
    addParameter(EffectParameter::makeDropdown("matchColors", "Match Colors", {"Hue & Chroma", "Hue"}, 0));
}

std::unique_ptr<Effect> LeaveColorEffect::clone() const {
    auto e = std::make_unique<LeaveColorEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void LeaveColorEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    Color target = getColorParam("colorToLeave");
    float tolerance = getFloatParam("tolerance") / 100.0f;
    float decolor = getFloatParam("amountToDecolor") / 100.0f;

    float tR = target.r, tG = target.g, tB = target.b;

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            float dr = p[0] - tR, dg = p[1] - tG, db = p[2] - tB;
            float dist = std::sqrt(dr * dr + dg * dg + db * db) / 255.0f;
            float match = std::clamp(1.0f - dist / (tolerance + 0.001f), 0.0f, 1.0f);
            float desat = match * decolor;

            float luma = 0.299f * p[0] + 0.587f * p[1] + 0.114f * p[2];
            p[0] = static_cast<uint8_t>(std::clamp(static_cast<double>(p[0] + (luma - p[0]) * desat), 0.0, 255.0));
            p[1] = static_cast<uint8_t>(std::clamp(static_cast<double>(p[1] + (luma - p[1]) * desat), 0.0, 255.0));
            p[2] = static_cast<uint8_t>(std::clamp(static_cast<double>(p[2] + (luma - p[2]) * desat), 0.0, 255.0));
        }
    }
}

} // namespace FreeEffect
