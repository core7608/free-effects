#include "../effect_registry.h"
#include "color_balance_effect.h"
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<ColorBalanceEffect> s_reg("Color Balance", "Color Correction");

ColorBalanceEffect::ColorBalanceEffect() {
    addParameter(EffectParameter::makeFloat("shadowsRed", "Shadows Red", -100.0, 100.0, 0.0));
    addParameter(EffectParameter::makeFloat("shadowsGreen", "Shadows Green", -100.0, 100.0, 0.0));
    addParameter(EffectParameter::makeFloat("shadowsBlue", "Shadows Blue", -100.0, 100.0, 0.0));
    addParameter(EffectParameter::makeFloat("midtonesRed", "Midtones Red", -100.0, 100.0, 0.0));
    addParameter(EffectParameter::makeFloat("midtonesGreen", "Midtones Green", -100.0, 100.0, 0.0));
    addParameter(EffectParameter::makeFloat("midtonesBlue", "Midtones Blue", -100.0, 100.0, 0.0));
    addParameter(EffectParameter::makeFloat("highlightsRed", "Highlights Red", -100.0, 100.0, 0.0));
    addParameter(EffectParameter::makeFloat("highlightsGreen", "Highlights Green", -100.0, 100.0, 0.0));
    addParameter(EffectParameter::makeFloat("highlightsBlue", "Highlights Blue", -100.0, 100.0, 0.0));
}

std::vector<ParameterGroup> ColorBalanceEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeFloat("shadowsRed", "Shadows Red", -100.0, 100.0, false),
        EffectParameter::makeFloat("shadowsGreen", "Shadows Green", -100.0, 100.0, false),
        EffectParameter::makeFloat("shadowsBlue", "Shadows Blue", -100.0, 100.0, false),
        EffectParameter::makeFloat("midtonesRed", "Midtones Red", -100.0, 100.0, false),
        EffectParameter::makeFloat("midtonesGreen", "Midtones Green", -100.0, 100.0, false),
        EffectParameter::makeFloat("midtonesBlue", "Midtones Blue", -100.0, 100.0, false),
        EffectParameter::makeFloat("highlightsRed", "Highlights Red", -100.0, 100.0, false),
        EffectParameter::makeFloat("highlightsGreen", "Highlights Green", -100.0, 100.0, false),
        EffectParameter::makeFloat("highlightsBlue", "Highlights Blue", -100.0, 100.0, false)
    }}};
}

std::unique_ptr<Effect> ColorBalanceEffect::clone() const {
    auto e = std::make_unique<ColorBalanceEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void ColorBalanceEffect::render(PixelBuffer& buffer, double time) {
    float sr = getFloatParam("shadowsRed") * 0.5f;
    float sg = getFloatParam("shadowsGreen") * 0.5f;
    float sb = getFloatParam("shadowsBlue") * 0.5f;
    float mr = getFloatParam("midtonesRed") * 0.5f;
    float mg = getFloatParam("midtonesGreen") * 0.5f;
    float mb = getFloatParam("midtonesBlue") * 0.5f;
    float hr = getFloatParam("highlightsRed") * 0.5f;
    float hg = getFloatParam("highlightsGreen") * 0.5f;
    float hb = getFloatParam("highlightsBlue") * 0.5f;

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            float lum = (p[0] * 0.299f + p[1] * 0.587f + p[2] * 0.114f) / 255.0f;
            float shadowW = std::max(0.0f, 1.0f - lum * 2.0f);
            float highlightW = std::max(0.0f, lum * 2.0f - 1.0f);
            float midtoneW = 1.0f - shadowW - highlightW;

            float r = p[0] + sr * shadowW + mr * midtoneW + hr * highlightW;
            float g = p[1] + sg * shadowW + mg * midtoneW + hg * highlightW;
            float b = p[2] + sb * shadowW + mb * midtoneW + hb * highlightW;
            p[0] = static_cast<uint8_t>(std::clamp(static_cast<double>(r), 0.0, 255.0));
            p[1] = static_cast<uint8_t>(std::clamp(static_cast<double>(g), 0.0, 255.0));
            p[2] = static_cast<uint8_t>(std::clamp(static_cast<double>(b), 0.0, 255.0));
        }
    }
}

} // namespace FreeEffect
