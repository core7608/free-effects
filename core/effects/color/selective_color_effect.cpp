#include "../effect_registry.h"
#include "selective_color_effect.h"
#include <cmath>
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<SelectiveColorEffect> s_reg("Selective Color", "Color");

SelectiveColorEffect::SelectiveColorEffect() {
    addParameter(EffectParameter::makeInt("color_select", "Color Select", 0, 8, 0));
    addParameter(EffectParameter::makeFloat("cyan", "Cyan", -100.0, 100.0, 0.0));
    addParameter(EffectParameter::makeFloat("magenta", "Magenta", -100.0, 100.0, 0.0));
    addParameter(EffectParameter::makeFloat("yellow", "Yellow", -100.0, 100.0, 0.0));
    addParameter(EffectParameter::makeFloat("black", "Black", -100.0, 100.0, 0.0));
}

std::vector<ParameterGroup> SelectiveColorEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeInt("color_select", "Color Select", 0, 8, 0),
        EffectParameter::makeFloat("cyan", "Cyan", -100.0, 100.0, 0.0),
        EffectParameter::makeFloat("magenta", "Magenta", -100.0, 100.0, 0.0),
        EffectParameter::makeFloat("yellow", "Yellow", -100.0, 100.0, 0.0),
        EffectParameter::makeFloat("black", "Black", -100.0, 100.0, 0.0)
    }}};
}

std::unique_ptr<Effect> SelectiveColorEffect::clone() const {
    auto e = std::make_unique<SelectiveColorEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void SelectiveColorEffect::render(PixelBuffer& buffer, double time) {
    if (buffer.width == 0 || buffer.height == 0) return;
    int colorSel = getIntParam("color_select");
    double cyanAdj = getFloatParam("cyan") * 0.01;
    double magentaAdj = getFloatParam("magenta") * 0.01;
    double yellowAdj = getFloatParam("yellow") * 0.01;
    double blackAdj = getFloatParam("black") * 0.01;
    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            double r = p[0] / 255.0;
            double g = p[1] / 255.0;
            double b = p[2] / 255.0;
            double maxC = std::max({r, g, b});
            double minC = std::min({r, g, b});
            double luma = r * 0.299 + g * 0.587 + b * 0.114;
            double hue = 0.0;
            if (maxC - minC > 0.001) {
                if (maxC == r) hue = std::fmod((g - b) / (maxC - minC), 6.0);
                else if (maxC == g) hue = (b - r) / (maxC - minC) + 2.0;
                else hue = (r - g) / (maxC - minC) + 4.0;
                hue *= 60.0;
                if (hue < 0) hue += 360.0;
            }
            bool match = false;
            switch (colorSel) {
                case 0: match = true; break;
                case 1: match = (hue >= 0 && hue < 60) || hue >= 300; break;
                case 2: match = hue >= 60 && hue < 120; break;
                case 3: match = hue >= 120 && hue < 180; break;
                case 4: match = hue >= 180 && hue < 240; break;
                case 5: match = hue >= 240 && hue < 300; break;
                case 6: match = luma > 0.66; break;
                case 7: match = luma > 0.33 && luma <= 0.66; break;
                case 8: match = luma <= 0.33; break;
            }
            if (!match) continue;
            double cFactor = 1.0 - cyanAdj;
            double mFactor = 1.0 - magentaAdj;
            double yFactor = 1.0 + yellowAdj;
            r = std::clamp(r * cFactor * mFactor * (1.0 + blackAdj * 0.5), 0.0, 1.0);
            g = std::clamp(g * cFactor * yFactor * (1.0 + blackAdj * 0.5), 0.0, 1.0);
            b = std::clamp(b * mFactor * yFactor * (1.0 + blackAdj * 0.5), 0.0, 1.0);
            p[0] = static_cast<uint8_t>(r * 255.0);
            p[1] = static_cast<uint8_t>(g * 255.0);
            p[2] = static_cast<uint8_t>(b * 255.0);
        }
    }
}

} // namespace FreeEffect
