#include "../effect_registry.h"
#include "card_wipe_effect.h"
#include <cmath>
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<CardWipeEffect> s_reg("Card Wipe", "Transition");

CardWipeEffect::CardWipeEffect() {
    addParameter(EffectParameter::makeInt("cards", "Cards", 2, 50, 5));
    addParameter(EffectParameter::makeInt("rows", "Rows", 1, 20, 3));
    addParameter(EffectParameter::makeDropdown("flipOrder", "Flip Order",
        {"Left to Right", "Right to Left", "Center Out"}, 0));
    addParameter(EffectParameter::makeFloat("tilt", "Tilt", 0.0, 90.0, 90.0));
    addParameter(EffectParameter::makeBool("backside", "Backside", true));
}

std::unique_ptr<Effect> CardWipeEffect::clone() const {
    auto e = std::make_unique<CardWipeEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void CardWipeEffect::render(PixelBuffer& buffer, double time) {
    if (buffer.width == 0 || buffer.height == 0) return;

    int numCards = std::max(getIntParam("cards"), 2);
    int numRows = std::max(getIntParam("rows"), 1);
    int flipOrder = getDropdownParam("flipOrder");
    double tilt = getFloatParam("tilt") * 3.14159265 / 180.0;
    bool showBack = getBoolParam("backside");

    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);

    double cardW = static_cast<double>(buffer.width) / numCards;
    double cardH = static_cast<double>(buffer.height) / numRows;

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            int col = static_cast<int>(x / cardW);
            int row = static_cast<int>(y / cardH);
            col = std::clamp(col, 0, numCards - 1);
            row = std::clamp(row, 0, numRows - 1);

            double progress;
            switch (flipOrder) {
                case 0: progress = static_cast<double>(col) / numCards; break;
                case 1: progress = 1.0 - static_cast<double>(col) / numCards; break;
                case 2: progress = 1.0 - std::abs(static_cast<double>(col) / numCards - 0.5) * 2.0; break;
                default: progress = 0.0;
            }

            double flipAngle = std::max(0.0, std::min(1.0, (time - progress) * 2.0)) * tilt;

            double localX = x - col * cardW;
            double centerX = cardW * 0.5;
            double dx = localX - centerX;

            if (flipAngle > 0.01) {
                double newDx = dx * std::cos(flipAngle);
                int srcX = std::clamp(static_cast<int>(centerX + newDx + col * cardW), 0, buffer.width - 1);

                bool flipped = std::cos(flipAngle) < 0;
                if (flipped && showBack) {
                    srcX = std::clamp(static_cast<int>(centerX - newDx + col * cardW), 0, buffer.width - 1);
                }

                int srcY = std::clamp(y, 0, buffer.height - 1);
                const uint8_t* src = buffer.pixelAt(srcX, srcY);
                uint8_t* dst = tmp.pixelAt(x, y);
                double darken = flipped ? 0.6 : 1.0;
                dst[0] = static_cast<uint8_t>(src[0] * darken);
                dst[1] = static_cast<uint8_t>(src[1] * darken);
                dst[2] = static_cast<uint8_t>(src[2] * darken);
                dst[3] = src[3];
            } else {
                const uint8_t* src = buffer.pixelAt(x, y);
                uint8_t* dst = tmp.pixelAt(x, y);
                dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3];
            }
        }
    }
    buffer.data = tmp.data;
}

} // namespace FreeEffect
