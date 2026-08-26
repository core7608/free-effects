#include "blend_modes.h"

namespace FreeEffect {

static inline uint8_t clampByte(double v) {
    return static_cast<uint8_t>(std::clamp(v, 0.0, 255.0));
}

HSL rgbToHsl(uint8_t ri, uint8_t gi, uint8_t bi) {
    double r = ri / 255.0;
    double g = gi / 255.0;
    double b = bi / 255.0;

    double cmax = std::max({r, g, b});
    double cmin = std::min({r, g, b});
    double delta = cmax - cmin;

    HSL hsl;
    hsl.l = (cmax + cmin) * 0.5;

    if (delta < 1e-7) {
        hsl.h = 0.0;
        hsl.s = 0.0;
    } else {
        hsl.s = delta / (1.0 - std::abs(2.0 * hsl.l - 1.0));
        if (hsl.s > 1.0) hsl.s = 1.0;

        if (cmax == r) {
            hsl.h = 60.0 * std::fmod((g - b) / delta, 6.0);
        } else if (cmax == g) {
            hsl.h = 60.0 * ((b - r) / delta + 2.0);
        } else {
            hsl.h = 60.0 * ((r - g) / delta + 4.0);
        }
        if (hsl.h < 0.0) hsl.h += 360.0;
    }

    return hsl;
}

void hslToRgb(const HSL& hsl, uint8_t& ro, uint8_t& go, uint8_t& bo) {
    double h = std::fmod(hsl.h, 360.0);
    if (h < 0.0) h += 360.0;
    double s = std::clamp(hsl.s, 0.0, 1.0);
    double l = std::clamp(hsl.l, 0.0, 1.0);

    double c = (1.0 - std::abs(2.0 * l - 1.0)) * s;
    double x = c * (1.0 - std::abs(std::fmod(h / 60.0, 2.0) - 1.0));
    double m = l - c * 0.5;

    double r = 0.0, g = 0.0, b = 0.0;
    if (h < 60.0)       { r = c; g = x; b = 0; }
    else if (h < 120.0) { r = x; g = c; b = 0; }
    else if (h < 180.0) { r = 0; g = c; b = x; }
    else if (h < 240.0) { r = 0; g = x; b = c; }
    else if (h < 300.0) { r = x; g = 0; b = c; }
    else                { r = c; g = 0; b = x; }

    ro = clampByte((r + m) * 255.0);
    go = clampByte((g + m) * 255.0);
    bo = clampByte((b + m) * 255.0);
}

static double calcColorBurn(double s, double d) {
    if (s <= 0.0) return 0.0;
    return std::max(0.0, 255.0 - 255.0 * (255.0 - d) / s);
}

static double calcColorDodge(double s, double d) {
    if (s >= 255.0) return 255.0;
    return std::min(255.0, 255.0 * d / (255.0 - s));
}

static double calcSoftLight(double s, double d) {
    if (s < 128.0) {
        return 2.0 * s * d / 255.0 + d * d * (255.0 - 2.0 * s) / 65025.0;
    } else {
        return 2.0 * d * (255.0 - s) / 255.0 +
               std::sqrt(d / 255.0) * (2.0 * s - 255.0) * std::sqrt(255.0 - d) / 255.0;
    }
}

void applyBlendMode(BlendModeType mode, const uint8_t src[4], uint8_t dst[4], double opacity) {
    double sR = src[0], sG = src[1], sB = src[2], sA = src[3] / 255.0;
    double dR = dst[0], dG = dst[1], dB = dst[2], dA = dst[3] / 255.0;

    double r = 0.0, g = 0.0, b = 0.0;

    switch (mode) {
        case BlendModeType::Normal:
            r = sR; g = sG; b = sB;
            break;

        case BlendModeType::Dissolve:
            r = sR; g = sG; b = sB;
            break;

        case BlendModeType::Darken:
            r = std::min(sR, dR);
            g = std::min(sG, dG);
            b = std::min(sB, dB);
            break;

        case BlendModeType::Multiply:
            r = sR * dR / 255.0;
            g = sG * dG / 255.0;
            b = sB * dB / 255.0;
            break;

        case BlendModeType::ColorBurn:
            r = calcColorBurn(sR, dR);
            g = calcColorBurn(sG, dG);
            b = calcColorBurn(sB, dB);
            break;

        case BlendModeType::LinearBurn:
            r = std::max(0.0, sR + dR - 255.0);
            g = std::max(0.0, sG + dG - 255.0);
            b = std::max(0.0, sB + dB - 255.0);
            break;

        case BlendModeType::DarkerColor: {
            double lumSrc = 0.299 * sR + 0.587 * sG + 0.114 * sB;
            double lumDst = 0.299 * dR + 0.587 * dG + 0.114 * dB;
            if (lumSrc <= lumDst) { r = sR; g = sG; b = sB; }
            else { r = dR; g = dG; b = dB; }
            break;
        }

        case BlendModeType::Lighten:
            r = std::max(sR, dR);
            g = std::max(sG, dG);
            b = std::max(sB, dB);
            break;

        case BlendModeType::Screen:
            r = 255.0 - (255.0 - sR) * (255.0 - dR) / 255.0;
            g = 255.0 - (255.0 - sG) * (255.0 - dG) / 255.0;
            b = 255.0 - (255.0 - sB) * (255.0 - dB) / 255.0;
            break;

        case BlendModeType::ColorDodge:
            r = calcColorDodge(sR, dR);
            g = calcColorDodge(sG, dG);
            b = calcColorDodge(sB, dB);
            break;

        case BlendModeType::LinearDodge:
            r = std::min(255.0, sR + dR);
            g = std::min(255.0, sG + dG);
            b = std::min(255.0, sB + dB);
            break;

        case BlendModeType::LighterColor: {
            double lumSrc = 0.299 * sR + 0.587 * sG + 0.114 * sB;
            double lumDst = 0.299 * dR + 0.587 * dG + 0.114 * dB;
            if (lumSrc >= lumDst) { r = sR; g = sG; b = sB; }
            else { r = dR; g = dG; b = dB; }
            break;
        }

        case BlendModeType::Overlay:
            if (dR < 128.0) r = 2.0 * sR * dR / 255.0;
            else r = 255.0 - 2.0 * (255.0 - sR) * (255.0 - dR) / 255.0;
            if (dG < 128.0) g = 2.0 * sG * dG / 255.0;
            else g = 255.0 - 2.0 * (255.0 - sG) * (255.0 - dG) / 255.0;
            if (dB < 128.0) b = 2.0 * sB * dB / 255.0;
            else b = 255.0 - 2.0 * (255.0 - sB) * (255.0 - dB) / 255.0;
            break;

        case BlendModeType::SoftLight:
            r = calcSoftLight(sR, dR);
            g = calcSoftLight(sG, dG);
            b = calcSoftLight(sB, dB);
            break;

        case BlendModeType::HardLight:
            if (sR < 128.0) r = 2.0 * sR * dR / 255.0;
            else r = 255.0 - 2.0 * (255.0 - sR) * (255.0 - dR) / 255.0;
            if (sG < 128.0) g = 2.0 * sG * dG / 255.0;
            else g = 255.0 - 2.0 * (255.0 - sG) * (255.0 - dG) / 255.0;
            if (sB < 128.0) b = 2.0 * sB * dB / 255.0;
            else b = 255.0 - 2.0 * (255.0 - sB) * (255.0 - dB) / 255.0;
            break;

        case BlendModeType::VividLight:
            r = sR < 128.0 ? calcColorBurn(sR * 2.0, dR) : calcColorDodge((sR - 128.0) * 2.0, dR);
            g = sG < 128.0 ? calcColorBurn(sG * 2.0, dG) : calcColorDodge((sG - 128.0) * 2.0, dG);
            b = sB < 128.0 ? calcColorBurn(sB * 2.0, dB) : calcColorDodge((sB - 128.0) * 2.0, dB);
            break;

        case BlendModeType::LinearLight:
            r = std::clamp(sR + 2.0 * dR - 255.0, 0.0, 255.0);
            g = std::clamp(sG + 2.0 * dG - 255.0, 0.0, 255.0);
            b = std::clamp(sB + 2.0 * dB - 255.0, 0.0, 255.0);
            break;

        case BlendModeType::PinLight:
            r = sR < 128.0 ? std::min(dR, 2.0 * sR) : std::max(dR, 2.0 * sR - 255.0);
            g = sG < 128.0 ? std::min(dG, 2.0 * sG) : std::max(dG, 2.0 * sG - 255.0);
            b = sB < 128.0 ? std::min(dB, 2.0 * sB) : std::max(dB, 2.0 * sB - 255.0);
            break;

        case BlendModeType::HardMix:
            r = (sR + dR >= 255.0) ? 255.0 : 0.0;
            g = (sG + dG >= 255.0) ? 255.0 : 0.0;
            b = (sB + dB >= 255.0) ? 255.0 : 0.0;
            break;

        case BlendModeType::Difference:
            r = std::abs(sR - dR);
            g = std::abs(sG - dG);
            b = std::abs(sB - dB);
            break;

        case BlendModeType::Exclusion:
            r = sR + dR - 2.0 * sR * dR / 255.0;
            g = sG + dG - 2.0 * sG * dG / 255.0;
            b = sB + dB - 2.0 * sB * dB / 255.0;
            break;

        case BlendModeType::Subtract:
            r = std::max(0.0, dR - sR);
            g = std::max(0.0, dG - sG);
            b = std::max(0.0, dB - sB);
            break;

        case BlendModeType::Divide:
            r = sR <= 0.0 ? 255.0 : std::min(255.0, 255.0 * dR / sR);
            g = sG <= 0.0 ? 255.0 : std::min(255.0, 255.0 * dG / sG);
            b = sB <= 0.0 ? 255.0 : std::min(255.0, 255.0 * dB / sB);
            break;

        case BlendModeType::Hue: {
            HSL srcHsl = rgbToHsl(src[0], src[1], src[2]);
            HSL dstHsl = rgbToHsl(dst[0], dst[1], dst[2]);
            dstHsl.h = srcHsl.h;
            hslToRgb(dstHsl, dst[0], dst[1], dst[2]);
            r = dst[0]; g = dst[1]; b = dst[2];
            break;
        }

        case BlendModeType::Saturation: {
            HSL srcHsl = rgbToHsl(src[0], src[1], src[2]);
            HSL dstHsl = rgbToHsl(dst[0], dst[1], dst[2]);
            dstHsl.s = srcHsl.s;
            hslToRgb(dstHsl, dst[0], dst[1], dst[2]);
            r = dst[0]; g = dst[1]; b = dst[2];
            break;
        }

        case BlendModeType::Color: {
            HSL srcHsl = rgbToHsl(src[0], src[1], src[2]);
            HSL dstHsl = rgbToHsl(dst[0], dst[1], dst[2]);
            dstHsl.h = srcHsl.h;
            dstHsl.s = srcHsl.s;
            hslToRgb(dstHsl, dst[0], dst[1], dst[2]);
            r = dst[0]; g = dst[1]; b = dst[2];
            break;
        }

        case BlendModeType::Luminosity: {
            HSL srcHsl = rgbToHsl(src[0], src[1], src[2]);
            HSL dstHsl = rgbToHsl(dst[0], dst[1], dst[2]);
            dstHsl.l = srcHsl.l;
            hslToRgb(dstHsl, dst[0], dst[1], dst[2]);
            r = dst[0]; g = dst[1]; b = dst[2];
            break;
        }
    }

    double a = sA * opacity;
    if (a <= 0.0) return;
    double invA = 1.0 - a;

    dst[0] = clampByte(r * a + dR * invA);
    dst[1] = clampByte(g * a + dG * invA);
    dst[2] = clampByte(b * a + dB * invA);
    dst[3] = clampByte((a + dA * invA) * 255.0);
}

BlendModeType mapBlendMode(BlendMode mode) {
    switch (mode) {
        case BlendMode::Normal:      return BlendModeType::Normal;
        case BlendMode::Add:         return BlendModeType::LinearDodge;
        case BlendMode::Multiply:    return BlendModeType::Multiply;
        case BlendMode::Screen:      return BlendModeType::Screen;
        case BlendMode::Darken:      return BlendModeType::Darken;
        case BlendMode::Lighten:     return BlendModeType::Lighten;
        case BlendMode::Overlay:     return BlendModeType::Overlay;
        case BlendMode::SoftLight:   return BlendModeType::SoftLight;
        case BlendMode::HardLight:   return BlendModeType::HardLight;
        case BlendMode::ColorBurn:   return BlendModeType::ColorBurn;
        case BlendMode::ColorDodge:  return BlendModeType::ColorDodge;
        case BlendMode::LinearBurn:  return BlendModeType::LinearBurn;
        case BlendMode::LinearDodge: return BlendModeType::LinearDodge;
        case BlendMode::VividLight:  return BlendModeType::VividLight;
        case BlendMode::LinearLight: return BlendModeType::LinearLight;
        case BlendMode::PinLight:    return BlendModeType::PinLight;
        case BlendMode::HardMix:     return BlendModeType::HardMix;
        case BlendMode::Difference:  return BlendModeType::Difference;
        case BlendMode::Exclusion:   return BlendModeType::Exclusion;
        case BlendMode::Subtract:    return BlendModeType::Subtract;
        case BlendMode::Divide:      return BlendModeType::Divide;
        case BlendMode::Hue:         return BlendModeType::Hue;
        case BlendMode::Saturation:  return BlendModeType::Saturation;
        case BlendMode::Color:       return BlendModeType::Color;
        case BlendMode::Luminosity:  return BlendModeType::Luminosity;
        default:                     return BlendModeType::Normal;
    }
}

} // namespace FreeEffect
