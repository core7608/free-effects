#include "../effect_registry.h"
#include "vegas_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<VegasEffect> s_reg("Vegas", "Generate");

VegasEffect::VegasEffect() {
    addParameter(EffectParameter::makeFloat("segments", "Segments", 1.0, 200.0, 20.0));
    addParameter(EffectParameter::makeFloat("maximumLength", "Maximum Length", 0.0, 200.0, 100.0));
    addParameter(EffectParameter::makeColor("color", "Color", {255.0, 255.0, 255.0, 1.0}));
    addParameter(EffectParameter::makeFloat("width", "Width", 1.0, 50.0, 3.0));
    addParameter(EffectParameter::makeFloat("softness", "Softness", 0.0, 100.0, 50.0));
}

std::unique_ptr<Effect> VegasEffect::clone() const {
    auto e = std::make_unique<VegasEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void VegasEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    float segs = getFloatParam("segments");
    float maxLen = getFloatParam("maximumLength") / 100.0f;
    Color col = getColorParam("color");
    float width = getFloatParam("width");
    float soft = getFloatParam("softness") / 100.0f;

    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);
    std::copy(buffer.data.begin(), buffer.data.end(), tmp.data.begin());

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            const uint8_t* p = tmp.pixelAt(x, y);
            float luma = (0.299f * p[0] + 0.587f * p[1] + 0.114f * p[2]) / 255.0f;
            if (luma > 0.3f) {
                float edgeDist = 0;
                bool isEdge = false;
                for (int dy = -1; dy <= 1 && !isEdge; dy++) {
                    for (int dx = -1; dx <= 1 && !isEdge; dx++) {
                        if (dx == 0 && dy == 0) continue;
                        int sx = std::clamp(x + dx, 0, buffer.width - 1);
                        int sy = std::clamp(y + dy, 0, buffer.height - 1);
                        const uint8_t* q = tmp.pixelAt(sx, sy);
                        float oluma = (0.299f * q[0] + 0.587f * q[1] + 0.114f * q[2]) / 255.0f;
                        if (std::abs(luma - oluma) > 0.2f) isEdge = true;
                    }
                }
                if (isEdge) {
                    uint8_t* dst = buffer.pixelAt(x, y);
                    dst[0] = static_cast<uint8_t>(col.r);
                    dst[1] = static_cast<uint8_t>(col.g);
                    dst[2] = static_cast<uint8_t>(col.b);
                }
            }
        }
    }
}

} // namespace FreeEffect
