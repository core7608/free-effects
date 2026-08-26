#include "../effect_registry.h"
#include "shadow_highlight_effect.h"
#include <cmath>
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<ShadowHighlightEffect> s_reg("Shadow/Highlight", "Color");

ShadowHighlightEffect::ShadowHighlightEffect() {
    addParameter(EffectParameter::makeFloat("shadow_amount", "Shadow Amount", 0.0, 100.0, 0.0));
    addParameter(EffectParameter::makeFloat("highlight_amount", "Highlight Amount", 0.0, 100.0, 0.0));
    addParameter(EffectParameter::makeFloat("tonal_width", "Tonal Width", 0.0, 1.0, 0.5));
    addParameter(EffectParameter::makeFloat("radius", "Radius", 1.0, 100.0, 30.0));
}

std::vector<ParameterGroup> ShadowHighlightEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeFloat("shadow_amount", "Shadow Amount", 0.0, 100.0, 0.0),
        EffectParameter::makeFloat("highlight_amount", "Highlight Amount", 0.0, 100.0, 0.0),
        EffectParameter::makeFloat("tonal_width", "Tonal Width", 0.0, 1.0, 0.5),
        EffectParameter::makeFloat("radius", "Radius", 1.0, 100.0, 30.0)
    }}};
}

std::unique_ptr<Effect> ShadowHighlightEffect::clone() const {
    auto e = std::make_unique<ShadowHighlightEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void ShadowHighlightEffect::render(PixelBuffer& buffer, double time) {
    if (buffer.width == 0 || buffer.height == 0) return;
    double shadowAmt = getFloatParam("shadow_amount") * 0.01;
    double highlightAmt = getFloatParam("highlight_amount") * 0.01;
    double tonalWidth = getFloatParam("tonal_width");
    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            double r = p[0] / 255.0;
            double g = p[1] / 255.0;
            double b = p[2] / 255.0;
            double luma = r * 0.299 + g * 0.587 + b * 0.114;
            double shadowMask = std::max(0.0, 1.0 - luma / tonalWidth);
            shadowMask = shadowMask * shadowMask;
            double highlightMask = std::max(0.0, (luma - (1.0 - tonalWidth)) / tonalWidth);
            highlightMask = highlightMask * highlightMask;
            double lift = shadowAmt * shadowMask * 0.5;
            double compress = 1.0 - highlightAmt * highlightMask * 0.5;
            r = std::clamp(r * compress + lift, 0.0, 1.0);
            g = std::clamp(g * compress + lift, 0.0, 1.0);
            b = std::clamp(b * compress + lift, 0.0, 1.0);
            p[0] = static_cast<uint8_t>(r * 255.0);
            p[1] = static_cast<uint8_t>(g * 255.0);
            p[2] = static_cast<uint8_t>(b * 255.0);
        }
    }
}

} // namespace FreeEffect
