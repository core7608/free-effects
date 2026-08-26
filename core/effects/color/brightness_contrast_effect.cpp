#include "../effect_registry.h"
#include "brightness_contrast_effect.h"
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<BrightnessContrastEffect> s_reg("Brightness & Contrast", "Color Correction");

BrightnessContrastEffect::BrightnessContrastEffect() {
    addParameter(EffectParameter::makeFloat("brightness", "Brightness", -100.0, 100.0, 0.0));
    addParameter(EffectParameter::makeFloat("contrast", "Contrast", -100.0, 100.0, 0.0));
}

std::vector<ParameterGroup> BrightnessContrastEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeFloat("brightness", "Brightness", -100.0, 100.0, false),
        EffectParameter::makeFloat("contrast", "Contrast", -100.0, 100.0, false)
    }}};
}

std::unique_ptr<Effect> BrightnessContrastEffect::clone() const {
    auto e = std::make_unique<BrightnessContrastEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void BrightnessContrastEffect::render(PixelBuffer& buffer, double time) {
    float brightness = getFloatParam("brightness") * 2.55f;
    float contrast = getFloatParam("contrast");
    float factor = (259.0f * (contrast + 255.0f)) / (255.0f * (259.0f - contrast));

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            for (int c = 0; c < 3; c++) {
                float v = static_cast<float>(p[c]) + brightness;
                v = factor * (v - 128.0f) + 128.0f;
                p[c] = static_cast<uint8_t>(std::clamp(static_cast<double>(v), 0.0, 255.0));
            }
        }
    }
}

} // namespace FreeEffect
