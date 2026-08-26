#include "color_management.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

ColorManager& ColorManager::instance() {
    static ColorManager mgr;
    return mgr;
}

void ColorManager::setWorkingSpace(ColorProfile profile) {
    m_workingSpace = profile;
}

ColorSpace ColorManager::getProfileData(ColorProfile profile) {
    ColorSpace cs;
    switch (profile) {
        case ColorProfile::sRGB:
        case ColorProfile::Rec709:
            cs.rX = 0.64; cs.rY = 0.33;
            cs.gX = 0.30; cs.gY = 0.60;
            cs.bX = 0.15; cs.bY = 0.06;
            cs.whiteX = 0.3127; cs.whiteY = 0.3290;
            cs.gamma = 2.2;
            cs.linearize = (profile == ColorProfile::sRGB);
            break;

        case ColorProfile::Rec2020:
            cs.rX = 0.708; cs.rY = 0.292;
            cs.gX = 0.170; cs.gY = 0.797;
            cs.bX = 0.131; cs.bY = 0.046;
            cs.whiteX = 0.3127; cs.whiteY = 0.3290;
            cs.gamma = 2.4;
            break;

        case ColorProfile::ACEScg:
            cs.rX = 0.713; cs.rY = 0.293;
            cs.gX = 0.165; cs.gY = 0.830;
            cs.bX = 0.128; cs.bY = 0.044;
            cs.whiteX = 0.32168; cs.whiteY = 0.33767;
            cs.gamma = 1.0;
            break;

        case ColorProfile::ACES2065:
            cs.rX = 0.7347; cs.rY = 0.2653;
            cs.gX = 0.0000; cs.gY = 1.0000;
            cs.bX = 0.0001; cs.bY = -0.0770;
            cs.whiteX = 0.32168; cs.whiteY = 0.33767;
            cs.gamma = 1.0;
            break;

        case ColorProfile::DisplayP3:
            cs.rX = 0.680; cs.rY = 0.320;
            cs.gX = 0.265; cs.gY = 0.690;
            cs.bX = 0.150; cs.bY = 0.060;
            cs.whiteX = 0.3127; cs.whiteY = 0.3290;
            cs.gamma = 2.2;
            cs.linearize = true;
            break;

        case ColorProfile::ProPhotoRGB:
            cs.rX = 0.7347; cs.rY = 0.2653;
            cs.gX = 0.1596; cs.gY = 0.8404;
            cs.bX = 0.0366; cs.bY = 0.0001;
            cs.whiteX = 0.3457; cs.whiteY = 0.3585;
            cs.gamma = 1.8;
            break;

        case ColorProfile::AdobeRGB:
            cs.rX = 0.6400; cs.rY = 0.3300;
            cs.gX = 0.2100; cs.gY = 0.7100;
            cs.bX = 0.1500; cs.bY = 0.0600;
            cs.whiteX = 0.3127; cs.whiteY = 0.3290;
            cs.gamma = 2.2;
            break;

        case ColorProfile::Linear:
            cs.rX = 0.64; cs.rY = 0.33;
            cs.gX = 0.30; cs.gY = 0.60;
            cs.bX = 0.15; cs.bY = 0.06;
            cs.whiteX = 0.3127; cs.whiteY = 0.3290;
            cs.gamma = 1.0;
            break;
    }
    return cs;
}

std::array<std::array<double, 3>, 3> ColorManager::computeRGBtoXYZ(const ColorSpace& cs) {
    double Xr = cs.rX / cs.rY;
    double Yr = 1.0;
    double Zr = (1.0 - cs.rX - cs.rY) / cs.rY;

    double Xg = cs.gX / cs.gY;
    double Yg = 1.0;
    double Zg = (1.0 - cs.gX - cs.gY) / cs.gY;

    double Xb = cs.bX / cs.bY;
    double Yb = 1.0;
    double Zb = (1.0 - cs.bX - cs.bY) / cs.bY;

    double Xw = cs.whiteX / cs.whiteY;
    double Yw = 1.0;
    double Zw = (1.0 - cs.whiteX - cs.whiteY) / cs.whiteY;

    double denom = Xr * (Yg * Zb - Yb * Zg) - Yr * (Xg * Zb - Xb * Zg) + Zr * (Xg * Yb - Xb * Yg);

    double Sr = (Xw * (Yg * Zb - Yb * Zg) - Yw * (Xg * Zb - Xb * Zg) + Zw * (Xg * Yb - Xb * Yg)) / denom;
    double Sg = (Xw * (Yr * Zb - Yb * Zr) - Yw * (Xr * Zb - Xb * Zr) + Zw * (Xr * Yb - Xb * Yr)) / (-denom);
    double Sb = (Xw * (Yr * Zg - Yg * Zr) - Yw * (Xr * Zg - Xg * Zr) + Zw * (Xr * Yg - Xg * Yr)) / denom;

    return {{
        {Sr * Xr, Sr * Yr, Sr * Zr},
        {Sg * Xg, Sg * Yg, Sg * Zg},
        {Sb * Xb, Sb * Yb, Sb * Zb}
    }};
}

std::array<std::array<double, 3>, 3> ColorManager::invertMatrix(
    const std::array<std::array<double, 3>, 3>& m) {
    double det = m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1])
               - m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0])
               + m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);

    if (std::abs(det) < 1e-12) {
        return {{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}}};
    }

    double invDet = 1.0 / det;
    return {{
        {(m[1][1] * m[2][2] - m[1][2] * m[2][1]) * invDet,
         (m[0][2] * m[2][1] - m[0][1] * m[2][2]) * invDet,
         (m[0][1] * m[1][2] - m[0][2] * m[1][1]) * invDet},
        {(m[1][2] * m[2][0] - m[1][0] * m[2][2]) * invDet,
         (m[0][0] * m[2][2] - m[0][2] * m[2][0]) * invDet,
         (m[0][2] * m[1][0] - m[0][0] * m[1][2]) * invDet},
        {(m[1][0] * m[2][1] - m[1][1] * m[2][0]) * invDet,
         (m[0][1] * m[2][0] - m[0][0] * m[2][1]) * invDet,
         (m[0][0] * m[1][1] - m[0][1] * m[1][0]) * invDet}
    }};
}

std::array<std::array<double, 3>, 3> ColorManager::multiplyMatrices(
    const std::array<std::array<double, 3>, 3>& a,
    const std::array<std::array<double, 3>, 3>& b) {
    std::array<std::array<double, 3>, 3> result = {};
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            for (int k = 0; k < 3; ++k) {
                result[i][j] += a[i][k] * b[k][j];
            }
        }
    }
    return result;
}

Color ColorManager::linearToGamma(const Color& c, double gamma) {
    Color result;
    result.a = c.a;
    auto toGamma = [gamma](double v) -> double {
        if (v <= 0.0) return 0.0;
        if (gamma == 2.2) {
            if (v <= 0.0031308) return 12.92 * v;
            return 1.055 * std::pow(v, 1.0 / 2.4) - 0.055;
        }
        return std::pow(v, 1.0 / gamma);
    };
    result.r = toGamma(c.r);
    result.g = toGamma(c.g);
    result.b = toGamma(c.b);
    return result;
}

Color ColorManager::gammaToLinear(const Color& c, double gamma) {
    Color result;
    result.a = c.a;
    auto toLinear = [gamma](double v) -> double {
        if (v <= 0.0) return 0.0;
        if (gamma == 2.2) {
            if (v <= 0.04045) return v / 12.92;
            return std::pow((v + 0.055) / 1.055, 2.4);
        }
        return std::pow(v, gamma);
    };
    result.r = toLinear(c.r);
    result.g = toLinear(c.g);
    result.b = toLinear(c.b);
    return result;
}

Color ColorManager::acesToneMap(const Color& c) {
    double a = 2.51;
    double b = 0.03;
    double d = 0.59;
    double e = 0.14;

    auto curve = [a, b, d, e](double x) -> double {
        x = std::max(0.0, x);
        return std::clamp((x * (a * x + b)) / (x * (d * x + e) + 0.006), 0.0, 1.0);
    };

    Color result;
    result.r = curve(c.r);
    result.g = curve(c.g);
    result.b = curve(c.b);
    result.a = c.a;
    return result;
}

Color ColorManager::convertColor(const Color& src, ColorProfile from, ColorProfile to) const {
    if (from == to) return src;

    ColorSpace fromCS = getProfileData(from);
    ColorSpace toCS = getProfileData(to);

    Color linearSrc = src;
    if (fromCS.linearize) {
        linearSrc = gammaToLinear(src, fromCS.gamma);
    } else if (fromCS.gamma != 1.0) {
        linearSrc = gammaToLinear(src, fromCS.gamma);
    }

    auto fromMat = computeRGBtoXYZ(fromCS);
    double xr = linearSrc.r * fromMat[0][0] + linearSrc.g * fromMat[1][0] + linearSrc.b * fromMat[2][0];
    double yr = linearSrc.r * fromMat[0][1] + linearSrc.g * fromMat[1][1] + linearSrc.b * fromMat[2][1];
    double zr = linearSrc.r * fromMat[0][2] + linearSrc.g * fromMat[1][2] + linearSrc.b * fromMat[2][2];

    auto toMat = computeRGBtoXYZ(toCS);
    auto toMatInv = invertMatrix(toMat);

    Color linearResult;
    linearResult.a = src.a;
    linearResult.r = xr * toMatInv[0][0] + yr * toMatInv[0][1] + zr * toMatInv[0][2];
    linearResult.g = xr * toMatInv[1][0] + yr * toMatInv[1][1] + zr * toMatInv[1][2];
    linearResult.b = xr * toMatInv[2][0] + yr * toMatInv[2][1] + zr * toMatInv[2][2];

    linearResult.r = std::max(0.0, linearResult.r);
    linearResult.g = std::max(0.0, linearResult.g);
    linearResult.b = std::max(0.0, linearResult.b);

    if (toCS.gamma != 1.0) {
        return linearToGamma(linearResult, toCS.gamma);
    }
    return linearResult;
}

} // namespace FreeEffect
