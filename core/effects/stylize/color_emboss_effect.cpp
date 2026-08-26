#include "../effect_registry.h"
#include "color_emboss_effect.h"
#include <cmath>
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<ColorEmbossEffect> s_reg("Color Emboss", "Stylize");

ColorEmbossEffect::ColorEmbossEffect() {
    addParameter(EffectParameter::makeAngle("direction", "Direction", 135.0));
    addParameter(EffectParameter::makeFloat("height", "Height", 0.0, 10.0, 2.0));
    addParameter(EffectParameter::makeFloat("contrast", "Contrast", 0.0, 5.0, 1.0));
}

std::vector<ParameterGroup> ColorEmbossEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeAngle("direction", "Direction", 135.0),
        EffectParameter::makeFloat("height", "Height", 0.0, 10.0, 2.0),
        EffectParameter::makeFloat("contrast", "Contrast", 0.0, 5.0, 1.0)
    }}};
}

std::unique_ptr<Effect> ColorEmbossEffect::clone() const {
    auto e = std::make_unique<ColorEmbossEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void ColorEmbossEffect::render(PixelBuffer& buffer, double time) {
    if (buffer.width == 0 || buffer.height == 0) return;
    double dir = getAngleParam("direction") * M_PI / 180.0;
    double h = getFloatParam("height");
    double contrast = getFloatParam("contrast");
    int dx = static_cast<int>(std::round(std::cos(dir)));
    int dy = static_cast<int>(std::round(std::sin(dir)));
    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);
    tmp.data = buffer.data;
    for (int y = 1; y < buffer.height - 1; y++) {
        for (int x = 1; x < buffer.width - 1; x++) {
            const uint8_t* c = tmp.pixelAt(x, y);
            const uint8_t* n = tmp.pixelAt(std::clamp(x + dx, 0, buffer.width - 1),
                                           std::clamp(y + dy, 0, buffer.height - 1));
            uint8_t* p = buffer.pixelAt(x, y);
            for (int ch = 0; ch < 3; ch++) {
                double diff = (c[ch] - n[ch]) * h;
                double val = 128.0 + diff * contrast;
                p[ch] = static_cast<uint8_t>(std::clamp(val, 0.0, 255.0));
            }
        }
    }
}

} // namespace FreeEffect
