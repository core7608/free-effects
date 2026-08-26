#include "../effect_registry.h"
#include "pixelate_transition.h"
#include <cmath>
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<PixelateTransition> s_reg("Pixelate Transition", "Transition");

PixelateTransition::PixelateTransition() {
    addParameter(EffectParameter::makeFloat("progress", "Progress", 0.0, 1.0, 0.0));
    addParameter(EffectParameter::makeInt("max_size", "Max Block Size", 4, 200, 64));
    addParameter(EffectParameter::makeBool("fade_out", "Fade Out", true));
}

std::vector<ParameterGroup> PixelateTransition::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeFloat("progress", "Progress", 0.0, 1.0, 0.0),
        EffectParameter::makeInt("max_size", "Max Block Size", 4, 200, 64),
        EffectParameter::makeBool("fade_out", "Fade Out", true)
    }}};
}

std::unique_ptr<Effect> PixelateTransition::clone() const {
    auto e = std::make_unique<PixelateTransition>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void PixelateTransition::render(PixelBuffer& buffer, double time) {
    if (buffer.width == 0 || buffer.height == 0) return;
    double progress = getFloatParam("progress");
    int maxSize = getIntParam("max_size");
    bool fadeOut = getBoolParam("fade_out");
    double pixSize = 1.0 + (maxSize - 1.0) * std::abs(std::sin(progress * 3.14159));
    int blockSize = std::max(1, static_cast<int>(pixSize));
    double fadeAlpha = fadeOut ? (1.0 - std::abs(progress * 2.0 - 1.0)) : 1.0;
    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);
    tmp.data = buffer.data;
    for (int y = 0; y < buffer.height; y += blockSize) {
        for (int x = 0; x < buffer.width; x += blockSize) {
            double r = 0, g = 0, b = 0, a = 0;
            int count = 0;
            for (int dy = 0; dy < blockSize && y + dy < buffer.height; dy++) {
                for (int dx = 0; dx < blockSize && x + dx < buffer.width; dx++) {
                    const uint8_t* p = tmp.pixelAt(x + dx, y + dy);
                    r += p[0];
                    g += p[1];
                    b += p[2];
                    a += p[3];
                    count++;
                }
            }
            if (count == 0) continue;
            r /= count;
            g /= count;
            b /= count;
            a /= count;
            for (int dy = 0; dy < blockSize && y + dy < buffer.height; dy++) {
                for (int dx = 0; dx < blockSize && x + dx < buffer.width; dx++) {
                    uint8_t* p = buffer.pixelAt(x + dx, y + dy);
                    p[0] = static_cast<uint8_t>(r * fadeAlpha);
                    p[1] = static_cast<uint8_t>(g * fadeAlpha);
                    p[2] = static_cast<uint8_t>(b * fadeAlpha);
                    p[3] = static_cast<uint8_t>(a * fadeAlpha);
                }
            }
        }
    }
}

} // namespace FreeEffect
