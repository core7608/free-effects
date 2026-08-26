#include "../effect_registry.h"
#include "color_profile_converter_effect.h"
#include <cmath>
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<ColorProfileConverterEffect> s_reg("Color Profile Converter", "Utility");

ColorProfileConverterEffect::ColorProfileConverterEffect() {
    addParameter(EffectParameter::makeDropdown("inputProfile", "Input Profile",
        {"sRGB", "Rec. 709", "Rec. 2020", "Adobe RGB", "ProPhoto RGB"}, 0));
    addParameter(EffectParameter::makeDropdown("outputProfile", "Output Profile",
        {"sRGB", "Rec. 709", "Rec. 2020", "Adobe RGB", "ProPhoto RGB"}, 0));
}

std::unique_ptr<Effect> ColorProfileConverterEffect::clone() const {
    auto e = std::make_unique<ColorProfileConverterEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

static void rgbToLinear(double r, double g, double b, double& lr, double& lg, double& lb, int profile) {
    switch (profile) {
        case 0: // sRGB
            lr = r <= 0.04045 ? r / 12.92 : std::pow((r + 0.055) / 1.055, 2.4);
            lg = g <= 0.04045 ? g / 12.92 : std::pow((g + 0.055) / 1.055, 2.4);
            lb = b <= 0.04045 ? b / 12.92 : std::pow((b + 0.055) / 1.055, 2.4);
            break;
        case 1: // Rec. 709 (same as sRGB)
            lr = r <= 0.081 ? r / 4.5 : std::pow((r + 0.099) / 1.099, 1.0 / 0.45);
            lg = g <= 0.081 ? g / 4.5 : std::pow((g + 0.099) / 1.099, 1.0 / 0.45);
            lb = b <= 0.081 ? b / 4.5 : std::pow((b + 0.099) / 1.099, 1.0 / 0.45);
            break;
        case 2: // Rec. 2020
            lr = r <= 0.081 ? r / 4.5 : std::pow((r + 0.099) / 1.099, 1.0 / 0.45);
            lg = g <= 0.081 ? g / 4.5 : std::pow((g + 0.099) / 1.099, 1.0 / 0.45);
            lb = b <= 0.081 ? b / 4.5 : std::pow((b + 0.099) / 1.099, 1.0 / 0.45);
            break;
        case 3: // Adobe RGB
            lr = std::pow(r, 2.19921875);
            lg = std::pow(g, 2.19921875);
            lb = std::pow(b, 2.19921875);
            break;
        case 4: // ProPhoto RGB
            lr = r <= 0.03928 ? r / 16.0 : std::pow((r + 0.055) / 1.055, 2.4);
            lg = g <= 0.03928 ? g / 16.0 : std::pow((g + 0.055) / 1.055, 2.4);
            lb = b <= 0.03928 ? b / 16.0 : std::pow((b + 0.055) / 1.055, 2.4);
            break;
        default:
            lr = r; lg = g; lb = b;
    }
}

static void linearToRgb(double lr, double lg, double lb, double& r, double& g, double& b, int profile) {
    switch (profile) {
        case 0: // sRGB
            r = lr <= 0.0031308 ? lr * 12.92 : 1.055 * std::pow(lr, 1.0 / 2.4) - 0.055;
            g = lg <= 0.0031308 ? lg * 12.92 : 1.055 * std::pow(lg, 1.0 / 2.4) - 0.055;
            b = lb <= 0.0031308 ? lb * 12.92 : 1.055 * std::pow(lb, 1.0 / 2.4) - 0.055;
            break;
        case 1: // Rec. 709
            r = lr <= 0.018 ? lr * 4.5 : 1.099 * std::pow(lr, 0.45) - 0.099;
            g = lg <= 0.018 ? lg * 4.5 : 1.099 * std::pow(lg, 0.45) - 0.099;
            b = lb <= 0.018 ? lb * 4.5 : 1.099 * std::pow(lb, 0.45) - 0.099;
            break;
        case 2: // Rec. 2020
            r = lr <= 0.018 ? lr * 4.5 : 1.099 * std::pow(lr, 0.45) - 0.099;
            g = lg <= 0.018 ? lg * 4.5 : 1.099 * std::pow(lg, 0.45) - 0.099;
            b = lb <= 0.018 ? lb * 4.5 : 1.099 * std::pow(lb, 0.45) - 0.099;
            break;
        case 3: // Adobe RGB
            r = std::pow(lr, 1.0 / 2.19921875);
            g = std::pow(lg, 1.0 / 2.19921875);
            b = std::pow(lb, 1.0 / 2.19921875);
            break;
        case 4: // ProPhoto RGB
            r = lr <= 0.001953 ? lr * 16.0 : 1.055 * std::pow(lr, 1.0 / 2.4) - 0.055;
            g = lg <= 0.001953 ? lg * 16.0 : 1.055 * std::pow(lg, 1.0 / 2.4) - 0.055;
            b = lb <= 0.001953 ? lb * 16.0 : 1.055 * std::pow(lb, 1.0 / 2.4) - 0.055;
            break;
        default:
            r = lr; g = lg; b = lb;
    }
}

void ColorProfileConverterEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    int inProfile = getDropdownParam("inputProfile");
    int outProfile = getDropdownParam("outputProfile");

    if (inProfile == outProfile) return;

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            double r = p[0] / 255.0;
            double g = p[1] / 255.0;
            double b = p[2] / 255.0;

            double lr, lg, lb;
            rgbToLinear(r, g, b, lr, lg, lb, inProfile);

            double or_, og, ob;
            linearToRgb(lr, lg, lb, or_, og, ob, outProfile);

            p[0] = static_cast<uint8_t>(std::clamp(or_ * 255.0, 0.0, 255.0));
            p[1] = static_cast<uint8_t>(std::clamp(og * 255.0, 0.0, 255.0));
            p[2] = static_cast<uint8_t>(std::clamp(ob * 255.0, 0.0, 255.0));
        }
    }
}

} // namespace FreeEffect
