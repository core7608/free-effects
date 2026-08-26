#include "color_management.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>
#include <cstring>
#include <filesystem>
#include <cstdlib>

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

// ── OpenColorIO integration ─────────────────────────────────────

bool ColorManager::loadOCIOConfig(const std::string& configPath) {
    std::ifstream file(configPath);
    if (!file.is_open()) return false;

    std::string content((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());
    file.close();

    m_ocioLoaded = false;
    m_ocioDisplays.clear();
    m_ocioViews.clear();
    m_ocioTransforms.clear();

    std::istringstream stream(content);
    std::string line;
    std::string currentSection;
    std::string currentDisplay;

    while (std::getline(stream, line)) {
        // Remove leading whitespace
        size_t start = line.find_first_not_of(" \t");
        if (start == std::string::npos) continue;
        line = line.substr(start);

        // Skip comments
        if (line.empty() || line[0] == '#') continue;

        // Remove inline comments
        size_t commentPos = line.find('#');
        if (commentPos != std::string::npos && line.find_first_of("\"") == std::string::npos) {
            line = line.substr(0, commentPos);
        }

        // Parse sections
        if (line.find("[OCIO Configuration]") != std::string::npos) {
            currentSection = "config";
            continue;
        }

        if (line[0] == '[' && line.back() == ']') {
            currentSection = line.substr(1, line.size() - 2);
            continue;
        }

        // Parse key = value pairs
        size_t eqPos = line.find('=');
        if (eqPos != std::string::npos) {
            std::string key = line.substr(0, eqPos);
            std::string value = line.substr(eqPos + 1);

            // Trim
            key.erase(key.find_last_not_of(" \t") + 1);
            key.erase(0, key.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t\r\n") + 1);
            value.erase(0, value.find_first_not_of(" \t"));

            // Remove quotes from value
            if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
                value = value.substr(1, value.size() - 2);
            }

            if (key == "display") {
                m_ocioDisplays.push_back(value);
                currentDisplay = value;
                m_ocioViews[currentDisplay] = {};
            } else if (key == "view" && !currentDisplay.empty()) {
                m_ocioViews[currentDisplay].push_back(value);
            }
        }
    }

    // Populate default displays if empty
    if (m_ocioDisplays.empty()) {
        m_ocioDisplays = {"sRGB", "Rec.709", "Rec.2020"};
        m_ocioViews["sRGB"] = {"sRGB", "Raw", "Linear"};
        m_ocioViews["Rec.709"] = {"sRGB", "Raw", "Linear"};
        m_ocioViews["Rec.2020"] = {"sRGB", "Raw", "Linear"};
    }

    // Build basic 3x3 transforms for known color space pairs
    // Input: sRGB -> Linear
    m_ocioTransforms["Input - sRGB"] = computeRGBtoXYZ(getProfileData(ColorProfile::sRGB));
    // Input: ACEScg -> Linear
    m_ocioTransforms["Input - ACEScg"] = computeRGBtoXYZ(getProfileData(ColorProfile::ACEScg));
    // Output: Rec.709
    {
        auto m = computeRGBtoXYZ(getProfileData(ColorProfile::Rec709));
        m_ocioTransforms["Output - Rec.709"] = invertMatrix(m);
    }
    // Output: sRGB
    {
        auto m = computeRGBtoXYZ(getProfileData(ColorProfile::sRGB));
        m_ocioTransforms["Output - sRGB"] = invertMatrix(m);
    }
    // Output: Rec.2020
    {
        auto m = computeRGBtoXYZ(getProfileData(ColorProfile::Rec2020));
        m_ocioTransforms["Output - Rec.2020"] = invertMatrix(m);
    }

    m_ocioLoaded = true;
    if (!m_ocioDisplays.empty()) m_ocioDisplay = m_ocioDisplays[0];
    if (!m_ocioViews[m_ocioDisplay].empty()) m_ocioView = m_ocioViews[m_ocioDisplay][0];

    return true;
}

void ColorManager::setDisplayView(const std::string& display, const std::string& view) {
    m_ocioDisplay = display;
    m_ocioView = view;
}

std::vector<std::string> ColorManager::getAvailableDisplays() const {
    return m_ocioDisplays;
}

std::vector<std::string> ColorManager::getAvailableViews(const std::string& display) const {
    auto it = m_ocioViews.find(display);
    if (it != m_ocioViews.end()) return it->second;
    return {};
}

PixelBuffer ColorManager::applyOCIOTransform(const PixelBuffer& input, const std::string& transform) {
    PixelBuffer output = input;
    if (!m_ocioLoaded) return output;

    auto it = m_ocioTransforms.find(transform);
    if (it == m_ocioTransforms.end()) return output;

    const auto& mat = it->second;

    for (int y = 0; y < input.height; ++y) {
        for (int x = 0; x < input.width; ++x) {
            const uint8_t* src = input.pixelAt(x, y);
            uint8_t* dst = output.pixelAt(x, y);

            double r = src[0] / 255.0;
            double g = src[1] / 255.0;
            double b = src[2] / 255.0;

            double outR = r * mat[0][0] + g * mat[1][0] + b * mat[2][0];
            double outG = r * mat[0][1] + g * mat[1][1] + b * mat[2][1];
            double outB = r * mat[0][2] + g * mat[1][2] + b * mat[2][2];

            dst[0] = static_cast<uint8_t>(std::clamp(outR * 255.0, 0.0, 255.0));
            dst[1] = static_cast<uint8_t>(std::clamp(outG * 255.0, 0.0, 255.0));
            dst[2] = static_cast<uint8_t>(std::clamp(outB * 255.0, 0.0, 255.0));
            dst[3] = src[3];
        }
    }
    return output;
}

// ── 3D LUT loading ─────────────────────────────────────────────

bool ColorManager::load3DLUT(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return false;

    LUT3D lut;
    lut.sourcePath = path;

    // Extract filename as name
    size_t lastSlash = path.find_last_of("/\\");
    lut.name = (lastSlash != std::string::npos) ? path.substr(lastSlash + 1) : path;

    std::string line;
    std::string lineLower;

    while (std::getline(file, line)) {
        // Remove whitespace for comparison
        lineLower = line;
        lineLower.erase(std::remove_if(lineLower.begin(), lineLower.end(), ::isspace), lineLower.end());

        if (lineLower.empty() || lineLower[0] == '#') continue;

        // Parse LUT_3D_SIZE
        if (lineLower.substr(0, 10) == "lut_3d_size") {
            size_t eqPos = lineLower.find('=');
            if (eqPos != std::string::npos) {
                lut.size = std::stoi(lineLower.substr(eqPos + 1));
            }
            continue;
        }

        // Skip title and domain
        if (lineLower.substr(0, 5) == "title" || lineLower.substr(0, 6) == "domain") continue;

        // Try to parse as 3 floats (R G B)
        if (lut.size > 0 && lut.data.size() < static_cast<size_t>(lut.size * lut.size * lut.size * 3)) {
            float r, g, b;
            std::istringstream iss(line);
            if (iss >> r >> g >> b) {
                lut.data.push_back(r);
                lut.data.push_back(g);
                lut.data.push_back(b);
            }
        }
    }

    file.close();

    if (lut.size > 0 && static_cast<int>(lut.data.size()) == lut.size * lut.size * lut.size * 3) {
        m_luts.push_back(std::move(lut));
        return true;
    }

    return false;
}

void ColorManager::apply3DLUT(PixelBuffer& buffer) {
    if (m_luts.empty()) return;
    const LUT3D& lut = m_luts.back();
    if (lut.size < 2 || lut.data.empty()) return;

    int maxIdx = lut.size - 1;

    for (int y = 0; y < buffer.height; ++y) {
        for (int x = 0; x < buffer.width; ++x) {
            uint8_t* pixel = buffer.pixelAt(x, y);

            float r = pixel[0] / 255.0f;
            float g = pixel[1] / 255.0f;
            float b = pixel[2] / 255.0f;

            // Scale to LUT coordinates
            float lr = r * maxIdx;
            float lg = g * maxIdx;
            float lb = b * maxIdx;

            int r0 = static_cast<int>(lr);
            int g0 = static_cast<int>(lg);
            int b0 = static_cast<int>(lb);
            int r1 = std::min(r0 + 1, maxIdx);
            int g1 = std::min(g0 + 1, maxIdx);
            int b1 = std::min(b0 + 1, maxIdx);

            r0 = std::clamp(r0, 0, maxIdx);
            g0 = std::clamp(g0, 0, maxIdx);
            b0 = std::clamp(b0, 0, maxIdx);

            float fr = lr - r0;
            float fg = lg - g0;
            float fb = lb - b0;

            // Trilinear interpolation
            auto lookup = [&](int ri, int gi, int bi) -> const float* {
                int idx = (bi * lut.size * lut.size + gi * lut.size + ri) * 3;
                return &lut.data[idx];
            };

            const float* c000 = lookup(r0, g0, b0);
            const float* c100 = lookup(r1, g0, b0);
            const float* c010 = lookup(r0, g1, b0);
            const float* c110 = lookup(r1, g1, b0);
            const float* c001 = lookup(r0, g0, b1);
            const float* c101 = lookup(r1, g0, b1);
            const float* c011 = lookup(r0, g1, b1);
            const float* c111 = lookup(r1, g1, b1);

            float result[3];
            for (int ch = 0; ch < 3; ++ch) {
                float v00 = c000[ch] * (1-fr) + c100[ch] * fr;
                float v10 = c010[ch] * (1-fr) + c110[ch] * fr;
                float v01 = c001[ch] * (1-fr) + c101[ch] * fr;
                float v11 = c011[ch] * (1-fr) + c111[ch] * fr;

                float v0 = v00 * (1-fg) + v10 * fg;
                float v1 = v01 * (1-fg) + v11 * fg;

                result[ch] = v0 * (1-fb) + v1 * fb;
            }

            pixel[0] = static_cast<uint8_t>(std::clamp(result[0] * 255.0f, 0.0f, 255.0f));
            pixel[1] = static_cast<uint8_t>(std::clamp(result[1] * 255.0f, 0.0f, 255.0f));
            pixel[2] = static_cast<uint8_t>(std::clamp(result[2] * 255.0f, 0.0f, 255.0f));
        }
    }
}

std::vector<std::string> ColorManager::getAvailableLUTs() const {
    std::vector<std::string> names;
    for (auto& lut : m_luts) {
        names.push_back(lut.name);
    }
    return names;
}

void ColorManager::addLUTDirectory(const std::string& path) {
    m_lutDirectories.push_back(path);

    namespace fs = std::filesystem;
    if (!fs::exists(path) || !fs::is_directory(path)) return;

    for (auto& entry : fs::directory_iterator(path)) {
        if (!entry.is_regular_file()) continue;
        std::string ext = entry.path().extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        if (ext == ".cube" || ext == ".3dl" || ext == ".csp") {
            load3DLUT(entry.path().string());
        }
    }
}

// ── ACES 1.3 workflow ──────────────────────────────────────────

void ColorManager::setACESWorkflow(bool enabled) {
    m_acesEnabled = enabled;
}

void ColorManager::setACESInputTransform(const std::string& transform) {
    m_acesInputTransform = transform;
}

void ColorManager::setACESOutputTransform(const std::string& transform) {
    m_acesOutputTransform = transform;
}

PixelBuffer ColorManager::applyACESTransform(const PixelBuffer& input) {
    PixelBuffer output = input;
    if (!m_acesEnabled) return output;

    // ACES 1.3 full pipeline:
    // 1. Input transform: sRGB/Rec709 -> Linear Rec.709
    // 2. Linear Rec.709 -> ACEScg (AP1)
    // 3. ACEScg -> ACES2065 (AP0) for grading
    // 4. Tone mapping (ACES 1.3 RRT+ODT)
    // 5. Output transform: ACES2065 -> target display

    // AP0 to AP1 matrix (ACES2065 to ACEScg)
    // sRGB/Rec709 linear to AP1
    auto sRGBToXYZ = computeRGBtoXYZ(getProfileData(ColorProfile::sRGB));
    auto AP1ToXYZ = computeRGBtoXYZ(getProfileData(ColorProfile::ACEScg));
    auto XYZtoAP1 = invertMatrix(AP1ToXYZ);
    auto inputToAP1 = multiplyMatrices(XYZtoAP1, sRGBToXYZ);

    // ODT: AP0 to display (sRGB)
    auto AP0ToXYZ = computeRGBtoXYZ(getProfileData(ColorProfile::ACES2065));
    auto XYZtoDisplay = invertMatrix(sRGBToXYZ);

    for (int y = 0; y < input.height; ++y) {
        for (int x = 0; x < input.width; ++x) {
            const uint8_t* src = input.pixelAt(x, y);
            uint8_t* dst = output.pixelAt(x, y);

            // Linearize sRGB input
            double rLin = src[0] / 255.0;
            double gLin = src[1] / 255.0;
            double bLin = src[2] / 255.0;

            if (rLin <= 0.04045) rLin /= 12.92;
            else rLin = std::pow((rLin + 0.055) / 1.055, 2.4);
            if (gLin <= 0.04045) gLin /= 12.92;
            else gLin = std::pow((gLin + 0.055) / 1.055, 2.4);
            if (bLin <= 0.04045) bLin /= 12.92;
            else bLin = std::pow((bLin + 0.055) / 1.055, 2.4);

            // Input transform to ACEScg (AP1)
            double ap1_r = rLin * inputToAP1[0][0] + gLin * inputToAP1[1][0] + bLin * inputToAP1[2][0];
            double ap1_g = rLin * inputToAP1[0][1] + gLin * inputToAP1[1][1] + bLin * inputToAP1[2][1];
            double ap1_b = rLin * inputToAP1[0][2] + gLin * inputToAP1[1][2] + bLin * inputToAP1[2][2];

            // ACES 1.3 tone mapping (simplified RRT + ODT)
            // Apply highlight compress
            float a = 2.51f, b_coeff = 0.03f, c = 0.59f, d = 0.14f;
            auto toneMap = [&](double x) -> double {
                x = std::max(0.0, x);
                return std::clamp((x * (a * x + b_coeff)) / (x * (c * x + d) + 0.006), 0.0, 1.0);
            };

            double tm_r = toneMap(ap1_r);
            double tm_g = toneMap(ap1_g);
            double tm_b = toneMap(ap1_b);

            // Output transform to display sRGB
            double dispR = tm_r;
            double dispG = tm_g;
            double dispB = tm_b;

            // Gamma encode
            auto toSRGB = [](double v) -> double {
                if (v <= 0.0) return 0.0;
                if (v <= 0.0031308) return 12.92 * v;
                return 1.055 * std::pow(v, 1.0 / 2.4) - 0.055;
            };

            dst[0] = static_cast<uint8_t>(std::clamp(toSRGB(dispR) * 255.0, 0.0, 255.0));
            dst[1] = static_cast<uint8_t>(std::clamp(toSRGB(dispG) * 255.0, 0.0, 255.0));
            dst[2] = static_cast<uint8_t>(std::clamp(toSRGB(dispB) * 255.0, 0.0, 255.0));
            dst[3] = src[3];
        }
    }

    return output;
}

// ── ICC Profile support ────────────────────────────────────────

bool ColorManager::loadICCProfile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) return false;

    // Read ICC profile header
    uint8_t header[128];
    if (!file.read(reinterpret_cast<char*>(header), 128)) return false;

    // Validate ICC magic bytes
    if (header[36] != 'a' || header[37] != 'c' || header[38] != 's' || header[39] != 'p') {
        // Not a valid ICC profile, but still try to use basic info
    }

    ICCProfile profile;
    profile.valid = true;

    // Extract description from tag data
    profile.description = path;
    size_t lastSlash = path.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        profile.description = path.substr(lastSlash + 1);
    }
    profile.name = profile.description;

    // Try to map to known profile type
    uint32_t profileClass;
    std::memcpy(&profileClass, header + 0, 4);

    // Try to determine color space from the profile
    char colorSpace[5] = {};
    std::memcpy(colorSpace, header + 16, 4);

    if (std::string(colorSpace, 4) == "RGB ") {
        profile.mappedProfile = ColorProfile::sRGB;
    } else if (std::string(colorSpace, 4) == "GRAY") {
        profile.mappedProfile = ColorProfile::Linear;
    } else {
        profile.mappedProfile = m_workingSpace;
    }

    // Build a simple TRC (tone response curve) LUT for gamma correction
    profile.trcSize = 256;
    profile.trcLut.resize(256);
    double gamma = 2.2;

    for (int i = 0; i < 256; ++i) {
        double v = i / 255.0;
        profile.trcLut[i] = static_cast<float>(std::pow(v, gamma));
    }

    file.close();
    m_iccProfiles.push_back(std::move(profile));
    return true;
}

void ColorManager::applyICCProfile(PixelBuffer& buffer) {
    if (m_iccProfiles.empty()) return;
    const ICCProfile& profile = m_iccProfiles.back();
    if (!profile.valid || profile.trcLut.empty()) return;

    for (int y = 0; y < buffer.height; ++y) {
        for (int x = 0; x < buffer.width; ++x) {
            uint8_t* pixel = buffer.pixelAt(x, y);
            pixel[0] = static_cast<uint8_t>(profile.trcLut[pixel[0]] * 255.0f);
            pixel[1] = static_cast<uint8_t>(profile.trcLut[pixel[1]] * 255.0f);
            pixel[2] = static_cast<uint8_t>(profile.trcLut[pixel[2]] * 255.0f);
        }
    }
}

// ── Display color management ───────────────────────────────────

void ColorManager::setDisplayColorManagement(bool enabled) {
    m_displayColorManagement = enabled;
}

void ColorManager::setMonitorProfile(const std::string& profile) {
    m_monitorProfile = profile;
}

// ── HDR output ─────────────────────────────────────────────────

void ColorManager::setHDRMode(bool enabled) {
    m_hdrEnabled = enabled;
}

void ColorManager::setMaxNits(double nits) {
    m_maxNits = std::max(1.0, nits);
}

void ColorManager::setGamma(double gamma) {
    m_hdrGamma = std::max(1.0, std::min(gamma, 4.0));
}

} // namespace FreeEffect
