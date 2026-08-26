#include "../effect_registry.h"
#include "scribble_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<ScribbleEffect> s_reg("Scribble", "Generate");

ScribbleEffect::ScribbleEffect() {
    addParameter(EffectParameter::makeColor("color", "Color", {0.0, 0.0, 0.0, 1.0}));
    addParameter(EffectParameter::makeFloat("strokeWidth", "Stroke Width", 1.0, 50.0, 3.0));
    addParameter(EffectParameter::makeInt("numStrokes", "Number of Strokes", 1, 200, 20));
    addParameter(EffectParameter::makeFloat("density", "Density", 0.0, 100.0, 50.0));
}

std::unique_ptr<Effect> ScribbleEffect::clone() const {
    auto e = std::make_unique<ScribbleEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void ScribbleEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    Color col = getColorParam("color");
    float sw = getFloatParam("strokeWidth");
    int ns = getIntParam("numStrokes");

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            const uint8_t* p = buffer.pixelAt(x, y);
            float luma = (0.299f * p[0] + 0.587f * p[1] + 0.114f * p[2]) / 255.0f;
            float hash = std::fmod(std::sin(static_cast<float>(x) * 12.9898f + y * 78.233f) * 43758.5453f, 1.0f);
            if (hash < 0.02f && luma > 0.1f) {
                uint8_t* dst = buffer.pixelAt(x, y);
                dst[0] = static_cast<uint8_t>(col.r);
                dst[1] = static_cast<uint8_t>(col.g);
                dst[2] = static_cast<uint8_t>(col.b);
                dst[3] = 255;
            }
        }
    }
}

} // namespace FreeEffect
