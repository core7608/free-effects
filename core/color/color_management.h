#pragma once
#include "../timeline/types.h"
#include "../rendering/renderer.h"
#include <string>
#include <array>
#include <vector>
#include <map>
#include <cstdint>

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

struct LUT3D {
    std::string name;
    int size = 0;
    std::vector<float> data;
    std::string sourcePath;
};

struct ICCProfile {
    std::string name;
    std::string description;
    ColorProfile mappedProfile;
    std::vector<float> trcLut;
    int trcSize = 0;
    bool valid = false;
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

    // OpenColorIO integration
    bool loadOCIOConfig(const std::string& configPath);
    void setDisplayView(const std::string& display, const std::string& view);
    std::vector<std::string> getAvailableDisplays() const;
    std::vector<std::string> getAvailableViews(const std::string& display) const;
    PixelBuffer applyOCIOTransform(const PixelBuffer& input, const std::string& transform);

    // 3D LUT loading
    bool load3DLUT(const std::string& path);
    void apply3DLUT(PixelBuffer& buffer);
    std::vector<std::string> getAvailableLUTs() const;
    void addLUTDirectory(const std::string& path);

    // ACES 1.3 workflow
    void setACESWorkflow(bool enabled);
    void setACESInputTransform(const std::string& transform);
    void setACESOutputTransform(const std::string& transform);
    PixelBuffer applyACESTransform(const PixelBuffer& input);

    // ICC Profile support
    bool loadICCProfile(const std::string& path);
    void applyICCProfile(PixelBuffer& buffer);

    // Display color management
    void setDisplayColorManagement(bool enabled);
    void setMonitorProfile(const std::string& profile);

    // HDR output
    void setHDRMode(bool enabled);
    void setMaxNits(double nits);
    void setGamma(double gamma);

private:
    ColorManager() = default;
    ColorProfile m_workingSpace = ColorProfile::sRGB;

    static std::array<std::array<double, 3>, 3> computeRGBtoXYZ(const ColorSpace& cs);
    static std::array<std::array<double, 3>, 3> invertMatrix(const std::array<std::array<double, 3>, 3>& m);
    static std::array<std::array<double, 3>, 3> multiplyMatrices(
        const std::array<std::array<double, 3>, 3>& a,
        const std::array<std::array<double, 3>, 3>& b);

    // OCIO state
    bool m_ocioLoaded = false;
    std::string m_ocioDisplay;
    std::string m_ocioView;
    std::vector<std::string> m_ocioDisplays;
    std::map<std::string, std::vector<std::string>> m_ocioViews;
    std::map<std::string, std::array<std::array<double, 3>, 3>> m_ocioTransforms;

    // 3D LUT state
    std::vector<LUT3D> m_luts;
    std::vector<std::string> m_lutDirectories;

    // ACES state
    bool m_acesEnabled = false;
    std::string m_acesInputTransform = "Input - sRGB";
    std::string m_acesOutputTransform = "Output - Rec.709";

    // ICC state
    std::vector<ICCProfile> m_iccProfiles;
    std::string m_monitorProfile;
    bool m_displayColorManagement = false;

    // HDR state
    bool m_hdrEnabled = false;
    double m_maxNits = 1000.0;
    double m_hdrGamma = 2.2;
};

} // namespace FreeEffect
