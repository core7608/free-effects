#include "../effect_registry.h"
#include "cc_calculator_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<CCCalculatorEffect> s_reg("CC Calculator", "Utility");

CCCalculatorEffect::CCCalculatorEffect() {
    addParameter(EffectParameter::makeDropdown("operation", "Operation", {"Add", "Subtract", "Multiply", "Divide", "Mod", "Power"}, 0));
    addParameter(EffectParameter::makeFloat("value1", "Value 1", -1000.0, 1000.0, 1.0));
    addParameter(EffectParameter::makeFloat("value2", "Value 2", -1000.0, 1000.0, 1.0));
    addParameter(EffectParameter::makeDropdown("channel", "Channel", {"RGBA", "RGB", "Alpha", "Red", "Green", "Blue"}, 0));
}

std::unique_ptr<Effect> CCCalculatorEffect::clone() const {
    auto e = std::make_unique<CCCalculatorEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void CCCalculatorEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    int op = getDropdownParam("operation");
    float v1 = getFloatParam("value1"), v2 = getFloatParam("value2");
    int ch = getDropdownParam("channel");

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            float result = 0;
            float input = (ch <= 2) ? p[0] : (ch == 3 ? p[0] : (ch == 4 ? p[1] : p[2]));
            switch (op) {
                case 0: result = input + v1 * v2; break;
                case 1: result = input - v1 * v2; break;
                case 2: result = input * v1 * v2; break;
                case 3: result = (v2 != 0) ? input / (v1 * v2) : 0; break;
                case 4: result = (v2 != 0) ? std::fmod(input, v2) : 0; break;
                case 5: result = std::pow(input / 255.0f, v2) * 255.0f; break;
            }
            result = std::clamp(result, 0.0f, 255.0f);
            if (ch == 0 || ch <= 2) { p[0] = p[1] = p[2] = static_cast<uint8_t>(result); }
            else if (ch == 3) p[0] = static_cast<uint8_t>(result);
            else if (ch == 4) p[1] = static_cast<uint8_t>(result);
            else if (ch == 5) p[2] = static_cast<uint8_t>(result);
            if (ch <= 2) p[3] = 255;
        }
    }
}

} // namespace FreeEffect
