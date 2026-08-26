#include "../effect_registry.h"
#include "card_dance_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<CardDanceEffect> s_reg("Card Dance", "Simulation");

CardDanceEffect::CardDanceEffect() {
    addParameter(EffectParameter::makeInt("gridRows", "Grid Rows", 1, 50, 5));
    addParameter(EffectParameter::makeInt("gridColumns", "Grid Columns", 1, 50, 5));
    addParameter(EffectParameter::makeFloat("cardScale", "Card Scale", 0.0, 200.0, 100.0));
    addParameter(EffectParameter::makeFloat("backsideGray", "Backside Gray", 0.0, 100.0, 100.0));
    addParameter(EffectParameter::makeFloat("bounceAmount", "Position Z", -1000.0, 1000.0, 0.0));
}

std::unique_ptr<Effect> CardDanceEffect::clone() const {
    auto e = std::make_unique<CardDanceEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void CardDanceEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    int rows = getIntParam("gridRows");
    int cols = getIntParam("gridColumns");
    float scale = getFloatParam("cardScale") / 100.0f;
    float bounce = getFloatParam("bounceAmount");
    float t = static_cast<float>(time) * 2.0f;

    int cardW = buffer.width / cols;
    int cardH = buffer.height / rows;
    if (cardW < 1 || cardH < 1) return;

    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);
    std::copy(buffer.data.begin(), buffer.data.end(), tmp.data.begin());

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* dst = buffer.pixelAt(x, y);
            dst[0] = dst[1] = dst[2] = 0; dst[3] = 0;
        }
    }

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            float offZ = std::sin(t + r * 0.5f + c * 0.3f) * bounce;
            float offY = std::cos(t * 0.7f + c * 0.4f) * bounce * 0.5f;
            int cw = static_cast<int>(cardW * scale);
            int ch = static_cast<int>(cardH * scale);
            int ox = c * cardW + cardW / 2 - cw / 2;
            int oy = r * cardH + cardH / 2 - ch / 2 + static_cast<int>(offY);

            for (int cy = 0; cy < ch; cy++) {
                for (int cx = 0; cx < cw; cx++) {
                    int dx = ox + cx, dy = oy + cy;
                    if (dx >= 0 && dx < buffer.width && dy >= 0 && dy < buffer.height) {
                        int srcX = c * cardW + cx * cardW / std::max(cw, 1);
                        int srcY = r * cardH + cy * cardH / std::max(ch, 1);
                        srcX = std::clamp(srcX, 0, buffer.width - 1);
                        srcY = std::clamp(srcY, 0, buffer.height - 1);
                        const uint8_t* src = tmp.pixelAt(srcX, srcY);
                        uint8_t* dst = buffer.pixelAt(dx, dy);
                        dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3];
                    }
                }
            }
        }
    }
}

} // namespace FreeEffect
