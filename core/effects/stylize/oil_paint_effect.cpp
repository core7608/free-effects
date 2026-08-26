#include "../effect_registry.h"
#include "oil_paint_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<OilPaintEffect> s_reg("Oil Paint", "Stylize");

OilPaintEffect::OilPaintEffect() {
    addParameter(EffectParameter::makeFloat("brushSize", "Brush Size", 1.0, 50.0, 5.0));
    addParameter(EffectParameter::makeFloat("cleanliness", "Cleanliness", 0.0, 100.0, 50.0));
    addParameter(EffectParameter::makeFloat("brushDetail", "Brush Detail", 0.0, 100.0, 50.0));
    addParameter(EffectParameter::makeFloat("scale", "Scale", 1.0, 5.0, 1.0));
}

std::unique_ptr<Effect> OilPaintEffect::clone() const {
    auto e = std::make_unique<OilPaintEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void OilPaintEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    int brushSize = static_cast<int>(getFloatParam("brushSize"));
    float cleanliness = getFloatParam("cleanliness") / 100.0f;
    int radius = std::max(brushSize, 1);

    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);
    std::copy(buffer.data.begin(), buffer.data.end(), tmp.data.begin());

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            int histR[256] = {}, histG[256] = {}, histB[256] = {};
            int count = 0;
            for (int dy = -radius; dy <= radius; dy++) {
                for (int dx = -radius; dx <= radius; dx++) {
                    int sx = std::clamp(x+dx, 0, buffer.width-1);
                    int sy = std::clamp(y+dy, 0, buffer.height-1);
                    const uint8_t* p = tmp.pixelAt(sx, sy);
                    histR[p[0]]++; histG[p[1]]++; histB[p[2]]++;
                    count++;
                }
            }
            auto modeVal = [](int* hist) -> uint8_t {
                int maxH = 0, val = 0;
                for (int i = 0; i < 256; i++) { if (hist[i] > maxH) { maxH = hist[i]; val = i; } }
                return static_cast<uint8_t>(val);
            };
            uint8_t* dst = buffer.pixelAt(x, y);
            dst[0] = modeVal(histR); dst[1] = modeVal(histG); dst[2] = modeVal(histB);
        }
    }
}

} // namespace FreeEffect
