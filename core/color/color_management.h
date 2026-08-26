#pragma once
#include "../timeline/types.h"
#include <string>
#include <array>

namespace FreeEffect {

enum class ColorProfile {
    sRGB, Rec709, Rec2020, ACEScg, ACES2065, DisplayP3, ProPhotoRGB, AdobeRGB, Linear
};

struct ColorSpace {
    double whiteX = 0.3127, whiteY = 0.3290;
    double rX = 0.64, rY = 0.33;
    double gX = 0.30, gY = 0.60;
    double bX = 0.15, bY = 0.06;
    double gamma = 2.2;
    bool linearize = false;
};

class ColorManager {
public:
    static ColorManager& instance();

    void setWorkingSpace(ColorProfile profile);
    ColorProfile getWorkingSpace() const { return m_workingSpace; }

    Color convertColor(const Color& src, ColorProfile from, ColorProfile to) const;

    static ColorSpace getProfileData(ColorProfile profile);
    static Color linearToGamma(const Color& c, double gamma = 2.2);
    static Color gammaToLinear(const Color& c, double gamma = 2.2);
    static Color acesToneMap(const Color& c);

private:
    ColorManager() = default;
    ColorProfile m_workingSpace = ColorProfile::sRGB;

    static std::array<std::array<double, 3>, 3> computeRGBtoXYZ(const ColorSpace& cs);
    static std::array<std::array<double, 3>, 3> invertMatrix(const std::array<std::array<double, 3>, 3>& m);
    static std::array<std::array<double, 3>, 3> multiplyMatrices(
        const std::array<std::array<double, 3>, 3>& a,
        const std::array<std::array<double, 3>, 3>& b);
};

} // namespace FreeEffect
