#include "../effect_registry.h"
#include "arithmetic_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<ArithmeticEffect> s_reg("Arithmetic", "Channel");

ArithmeticEffect::ArithmeticEffect() {
    addParameter(EffectParameter::makeDropdown("operation", "Operation",
        {"Add", "Subtract", "Multiply", "Divide", "And", "Or", "Xor", "Min", "Max"}, 0));
    addParameter(EffectParameter::makeFloat("overflow", "Overflow", 0.0, 255.0, 0.0));
}

std::unique_ptr<Effect> ArithmeticEffect::clone() const {
    auto e = std::make_unique<ArithmeticEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void ArithmeticEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    int op = getDropdownParam("operation");
    float overflow = getFloatParam("overflow");

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            float luma = (0.299f * p[0] + 0.587f * p[1] + 0.114f * p[2]);

            for (int c = 0; c < 3; c++) {
                float a = p[c] / 255.0f;
                float b = luma / 255.0f;
                float result = 0;

                switch (op) {
                    case 0: result = a + b; break;
                    case 1: result = a - b; break;
                    case 2: result = a * b; break;
                    case 3: result = (b > 0.001f) ? a / b : 1.0f; break;
                    case 4: result = static_cast<int>(a * 255) & static_cast<int>(b * 255) ? 1.0f : 0.0f; break;
                    case 5: result = (static_cast<int>(a * 255) | static_cast<int>(b * 255)) ? 1.0f : 0.0f; break;
                    case 6: result = (static_cast<int>(a * 255) ^ static_cast<int>(b * 255)) ? 1.0f : 0.0f; break;
                    case 7: result = std::min(a, b); break;
                    case 8: result = std::max(a, b); break;
                }

                p[c] = static_cast<uint8_t>(std::clamp(result * 255.0f, 0.0f, 255.0f));
            }
        }
    }
}

} // namespace FreeEffect
