#include "../effect_registry.h"
#include "cc_composite_effect.h"
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<CCCompositeEffect> s_reg("CC Composite", "Channel");

CCCompositeEffect::CCCompositeEffect() {
    addParameter(EffectParameter::makeDropdown("blendMode", "Blend Mode",
        {"Add", "Multiply", "Screen", "Overlay", "Normal"}, 0));
    addParameter(EffectParameter::makeFloat("opacity", "Opacity", 0.0, 100.0, 100.0));
    addParameter(EffectParameter::makeVec2("center", "Center", {0.0, 0.0}));
}

std::unique_ptr<Effect> CCCompositeEffect::clone() const {
    auto e = std::make_unique<CCCompositeEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void CCCompositeEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    int blendMode = getDropdownParam("blendMode");
    double opacity = getFloatParam("opacity") / 100.0;
    Vec2 center = getVec2Param("center");
    int offX = static_cast<int>(center.x);
    int offY = static_cast<int>(center.y);

    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);
    tmp.data = buffer.data;

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            const uint8_t* src = buffer.pixelAt(x, y);
            uint8_t* dst = tmp.pixelAt(x, y);
            int sx = std::clamp(x + offX, 0, buffer.width - 1);
            int sy = std::clamp(y + offY, 0, buffer.height - 1);
            const uint8_t* fg = buffer.pixelAt(sx, sy);

            double r1 = dst[0], g1 = dst[1], b1 = dst[2];
            double r2 = fg[0], g2 = fg[1], b2 = fg[2];
            double rn, gn, bn;

            switch (blendMode) {
                case 0: // Add
                    rn = std::min(r1 + r2, 255.0);
                    gn = std::min(g1 + g2, 255.0);
                    bn = std::min(b1 + b2, 255.0);
                    break;
                case 1: // Multiply
                    rn = r1 * r2 / 255.0;
                    gn = g1 * g2 / 255.0;
                    bn = b1 * b2 / 255.0;
                    break;
                case 2: // Screen
                    rn = 255.0 - (255.0 - r1) * (255.0 - r2) / 255.0;
                    gn = 255.0 - (255.0 - g1) * (255.0 - g2) / 255.0;
                    bn = 255.0 - (255.0 - b1) * (255.0 - b2) / 255.0;
                    break;
                case 3: // Overlay
                    rn = r1 < 128.0 ? 2.0 * r1 * r2 / 255.0 : 255.0 - 2.0 * (255.0 - r1) * (255.0 - r2) / 255.0;
                    gn = g1 < 128.0 ? 2.0 * g1 * g2 / 255.0 : 255.0 - 2.0 * (255.0 - g1) * (255.0 - g2) / 255.0;
                    bn = b1 < 128.0 ? 2.0 * b1 * b2 / 255.0 : 255.0 - 2.0 * (255.0 - b1) * (255.0 - b2) / 255.0;
                    break;
                default: // Normal
                    rn = r2; gn = g2; bn = b2;
                    break;
            }

            dst[0] = static_cast<uint8_t>(std::clamp(r1 + (rn - r1) * opacity, 0.0, 255.0));
            dst[1] = static_cast<uint8_t>(std::clamp(g1 + (gn - g1) * opacity, 0.0, 255.0));
            dst[2] = static_cast<uint8_t>(std::clamp(b1 + (bn - b1) * opacity, 0.0, 255.0));
        }
    }
    buffer.data = tmp.data;
}

} // namespace FreeEffect
