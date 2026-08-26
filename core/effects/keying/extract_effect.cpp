#include "../effect_registry.h"
#include "extract_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<ExtractEffect> s_reg("Extract", "Keying");

ExtractEffect::ExtractEffect() {
    addParameter(EffectParameter::makeDropdown("channel", "Channel", {"Luminance", "Red", "Green", "Blue", "Alpha"}, 0));
    addParameter(EffectParameter::makeFloat("blackPoint", "Black Point", 0.0, 255.0, 0.0));
    addParameter(EffectParameter::makeFloat("whitePoint", "White Point", 0.0, 255.0, 255.0));
    addParameter(EffectParameter::makeFloat("softness", "Softness", 0.0, 100.0, 0.0));
}

std::unique_ptr<Effect> ExtractEffect::clone() const {
    auto e = std::make_unique<ExtractEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void ExtractEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    int ch = getDropdownParam("channel");
    float black = getFloatParam("blackPoint");
    float white = getFloatParam("whitePoint");
    float soft = getFloatParam("softness");

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            float val = 0;
            if (ch == 0) val = 0.299f * p[0] + 0.587f * p[1] + 0.114f * p[2];
            else val = p[ch - 1];
            float alpha = std::clamp((val - black) / (white - black + 0.001f), 0.0f, 1.0f);
            float edge = soft > 0 ? std::clamp((alpha - 0.5f + soft * 0.5f) / (soft + 0.001f), 0.0f, 1.0f) : (alpha > 0.5f ? 1.0f : 0.0f);
            p[3] = static_cast<uint8_t>(std::clamp(edge * 255.0f, 0.0f, 255.0f));
        }
    }
}

} // namespace FreeEffect
