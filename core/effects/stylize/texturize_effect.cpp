#include "../effect_registry.h"
#include "texturize_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<TexturizeEffect> s_reg("Texturize", "Stylize");

TexturizeEffect::TexturizeEffect() {
    addParameter(EffectParameter::makeFloat("contrast", "Contrast", 0.0, 100.0, 50.0));
    addParameter(EffectParameter::makeFloat("depth", "Depth", 0.0, 100.0, 50.0));
    addParameter(EffectParameter::makeAngle("lightDirection", "Light Direction", -45.0));
}

std::unique_ptr<Effect> TexturizeEffect::clone() const {
    auto e = std::make_unique<TexturizeEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void TexturizeEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    float contrast = getFloatParam("contrast") / 100.0f;
    float depth = getFloatParam("depth") / 100.0f;
    float lightDir = getFloatParam("lightDirection") * 3.14159265f / 180.0f;

    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);
    std::copy(buffer.data.begin(), buffer.data.end(), tmp.data.begin());

    for (int y = 1; y < buffer.height - 1; y++) {
        for (int x = 1; x < buffer.width - 1; x++) {
            const uint8_t* c = tmp.pixelAt(x, y);
            float lumC = (0.299f*c[0] + 0.587f*c[1] + 0.114f*c[2]) / 255.0f;
            const uint8_t* l = tmp.pixelAt(x-1, y);
            const uint8_t* r2 = tmp.pixelAt(x+1, y);
            const uint8_t* t = tmp.pixelAt(x, y-1);
            const uint8_t* b2 = tmp.pixelAt(x, y+1);
            float dX = ((0.299f*r2[0]+0.587f*r2[1]+0.114f*r2[2]) - (0.299f*l[0]+0.587f*l[1]+0.114f*l[2])) / 255.0f;
            float dY = ((0.299f*b2[0]+0.587f*b2[1]+0.114f*b2[2]) - (0.299f*t[0]+0.587f*t[1]+0.114f*t[2])) / 255.0f;
            float shade = dX * std::cos(lightDir) + dY * std::sin(lightDir);
            shade = shade * depth * 2.0f;
            uint8_t* dst = buffer.pixelAt(x, y);
            for (int ch = 0; ch < 3; ch++) {
                float v = c[ch] / 255.0f;
                v = v + shade * (1.0f - std::abs(v - 0.5f) * 2.0f) * contrast;
                dst[ch] = static_cast<uint8_t>(std::clamp(v * 255.0f, 0.0f, 255.0f));
            }
        }
    }
}

} // namespace FreeEffect
