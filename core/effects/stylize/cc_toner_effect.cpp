#include "../effect_registry.h"
#include "cc_toner_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<CCTonerEffect> s_reg("CC Toner", "Stylize");

CCTonerEffect::CCTonerEffect() {
    addParameter(EffectParameter::makeDropdown("mapping", "Map Tones To", {"Midtones", "Highlights", "Shadows"}, 0));
    addParameter(EffectParameter::makeColor("color1", "Color 1", {255.0, 0.0, 0.0, 1.0}));
    addParameter(EffectParameter::makeColor("color2", "Color 2", {0.0, 0.0, 255.0, 1.0}));
    addParameter(EffectParameter::makeColor("color3", "Color 3", {255.0, 255.0, 0.0, 1.0}));
    addParameter(EffectParameter::makeBool("use3Colors", "Use 3 Colors", true));
}

std::unique_ptr<Effect> CCTonerEffect::clone() const {
    auto e = std::make_unique<CCTonerEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void CCTonerEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    Color c1 = getColorParam("color1"), c2 = getColorParam("color2"), c3 = getColorParam("color3");
    bool use3 = getBoolParam("use3Colors");

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            float luma = (0.299f * p[0] + 0.587f * p[1] + 0.114f * p[2]) / 255.0f;
            float r, g, b;
            if (!use3) {
                r = c1.r + (c2.r - c1.r) * luma;
                g = c1.g + (c2.g - c1.g) * luma;
                b = c1.b + (c2.b - c1.b) * luma;
            } else {
                if (luma < 0.5f) { float t = luma * 2.0f; r = c1.r+(c2.r-c1.r)*t; g = c1.g+(c2.g-c1.g)*t; b = c1.b+(c2.b-c1.b)*t; }
                else { float t = (luma-0.5f)*2.0f; r = c2.r+(c3.r-c2.r)*t; g = c2.g+(c3.g-c2.g)*t; b = c2.b+(c3.b-c2.b)*t; }
            }
            p[0] = static_cast<uint8_t>(std::clamp(r, 0.0f, 255.0f));
            p[1] = static_cast<uint8_t>(std::clamp(g, 0.0f, 255.0f));
            p[2] = static_cast<uint8_t>(std::clamp(b, 0.0f, 255.0f));
        }
    }
}

} // namespace FreeEffect
