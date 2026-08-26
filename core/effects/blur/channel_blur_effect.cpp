#include "../effect_registry.h"
#include "channel_blur_effect.h"
#include <algorithm>
#include <cmath>
#include <vector>

namespace FreeEffect {

static EffectRegistrar<ChannelBlurEffect> s_reg("Channel Blur", "Blur & Sharpen");

ChannelBlurEffect::ChannelBlurEffect() {
    addParameter(EffectParameter::makeInt("redBlurriness", "Red Blurriness", 0, 200, 0));
    addParameter(EffectParameter::makeInt("greenBlurriness", "Green Blurriness", 0, 200, 0));
    addParameter(EffectParameter::makeInt("blueBlurriness", "Blue Blurriness", 0, 200, 0));
    addParameter(EffectParameter::makeInt("alphaBlurriness", "Alpha Blurriness", 0, 200, 0));
    addParameter(EffectParameter::makeBool("repeatEdgePixels", "Repeat Edge Pixels", true));
}

std::unique_ptr<Effect> ChannelBlurEffect::clone() const {
    auto e = std::make_unique<ChannelBlurEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void ChannelBlurEffect::blurChannel(std::vector<uint8_t>& data, int w, int h, int ch, int radius) {
    if (radius <= 0) return;
    std::vector<float> line(w);
    for (int i = 0; i < w; i++) {
        line[i] = data[(0 * w + i) * 4 + ch];
    }
    std::vector<float> result(w * h);

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            float sum = 0;
            int count = 0;
            for (int dx = -radius; dx <= radius; dx++) {
                int sx = std::clamp(x + dx, 0, w - 1);
                sum += data[(y * w + sx) * 4 + ch];
                count++;
            }
            result[y * w + x] = sum / count;
        }
    }

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            float sum = 0;
            int count = 0;
            for (int dy = -radius; dy <= radius; dy++) {
                int sy = std::clamp(y + dy, 0, h - 1);
                sum += result[sy * w + x];
                count++;
            }
            data[(y * w + x) * 4 + ch] = static_cast<uint8_t>(std::clamp(static_cast<double>(sum / count), 0.0, 255.0));
        }
    }
}

void ChannelBlurEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    int rBlur = getIntParam("redBlurriness");
    int gBlur = getIntParam("greenBlurriness");
    int bBlur = getIntParam("blueBlurriness");
    int aBlur = getIntParam("alphaBlurriness");

    if (rBlur > 0) blurChannel(buffer.data, buffer.width, buffer.height, 0, rBlur);
    if (gBlur > 0) blurChannel(buffer.data, buffer.width, buffer.height, 1, gBlur);
    if (bBlur > 0) blurChannel(buffer.data, buffer.width, buffer.height, 2, bBlur);
    if (aBlur > 0) blurChannel(buffer.data, buffer.width, buffer.height, 3, aBlur);
}

} // namespace FreeEffect
