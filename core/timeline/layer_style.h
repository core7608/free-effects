#pragma once

#include "types.h"
#include <vector>

namespace FreeEffect {

struct PixelBuffer;

struct DropShadowStyle {
    bool enabled = false;
    Color color{0, 0, 0, 0.75};
    double opacity = 75.0;
    double angle = 120.0;
    double distance = 5.0;
    double spread = 0.0;
    double size = 5.0;
    bool useGlobalLight = true;
    int blendMode = 0;
};

struct InnerShadowStyle {
    bool enabled = false;
    Color color{0, 0, 0, 0.75};
    double opacity = 75.0;
    double angle = 120.0;
    double distance = 5.0;
    double choke = 0.0;
    double size = 5.0;
    bool useGlobalLight = true;
    int blendMode = 0;
};

struct OuterGlowStyle {
    bool enabled = false;
    Color color{1, 1, 1, 0.75};
    double opacity = 75.0;
    double spread = 0.0;
    double size = 5.0;
    int blendMode = 8;
    int technique = 0;
    int source = 0;
    double range = 50.0;
    double jitter = 0.0;
};

struct InnerGlowStyle {
    bool enabled = false;
    Color color{1, 1, 1, 0.75};
    double opacity = 75.0;
    double spread = 0.0;
    double size = 5.0;
    int blendMode = 8;
    int technique = 0;
    int source = 1;
    double range = 50.0;
    double jitter = 0.0;
};

struct BevelEmbossStyle {
    bool enabled = false;
    int style = 0;
    int technique = 0;
    double depth = 100.0;
    double size = 5.0;
    double soften = 0.0;
    double angle = 120.0;
    double altitude = 30.0;
    bool useGlobalLight = true;
    Color highlightColor{1, 1, 1, 0.75};
    int highlightMode = 9;
    Color shadowColor{0, 0, 0, 0.75};
    int shadowMode = 0;
};

struct StrokeStyle {
    bool enabled = false;
    double size = 3.0;
    int position = 0;
    Color color{0, 0, 0, 1.0};
    int blendMode = 0;
    double opacity = 100.0;
    int fillType = 0;
};

struct SatinStyle {
    bool enabled = false;
    Color color{0, 0, 0, 0.75};
    double opacity = 50.0;
    double angle = 19.0;
    double distance = 25.0;
    int blendMode = 0;
    double size = 10.0;
    bool invert = false;
};

struct ColorOverlayStyle {
    bool enabled = false;
    Color color{1, 0, 0, 1.0};
    int blendMode = 0;
    double opacity = 100.0;
};

struct GradientOverlayStyle {
    bool enabled = false;
    int blendMode = 0;
    double opacity = 100.0;
    int gradientType = 0;
    double angle = 90.0;
    double scale = 100.0;
    bool reverse = false;
    bool dither = false;
    Color color1{0, 0, 0, 1.0};
    Color color2{1, 1, 1, 1.0};
};

class LayerStyle {
public:
    DropShadowStyle& dropShadow() { return m_dropShadow; }
    InnerShadowStyle& innerShadow() { return m_innerShadow; }
    OuterGlowStyle& outerGlow() { return m_outerGlow; }
    InnerGlowStyle& innerGlow() { return m_innerGlow; }
    BevelEmbossStyle& bevelEmboss() { return m_bevelEmboss; }
    StrokeStyle& stroke() { return m_stroke; }
    SatinStyle& satin() { return m_satin; }
    ColorOverlayStyle& colorOverlay() { return m_colorOverlay; }
    GradientOverlayStyle& gradientOverlay() { return m_gradientOverlay; }

    const DropShadowStyle& dropShadow() const { return m_dropShadow; }
    const InnerShadowStyle& innerShadow() const { return m_innerShadow; }
    const OuterGlowStyle& outerGlow() const { return m_outerGlow; }
    const InnerGlowStyle& innerGlow() const { return m_innerGlow; }
    const BevelEmbossStyle& bevelEmboss() const { return m_bevelEmboss; }
    const StrokeStyle& stroke() const { return m_stroke; }
    const SatinStyle& satin() const { return m_satin; }
    const ColorOverlayStyle& colorOverlay() const { return m_colorOverlay; }
    const GradientOverlayStyle& gradientOverlay() const { return m_gradientOverlay; }

    bool hasAnyStyle() const;
    void renderStyles(PixelBuffer& buffer, int width, int height) const;

private:
    DropShadowStyle m_dropShadow;
    InnerShadowStyle m_innerShadow;
    OuterGlowStyle m_outerGlow;
    InnerGlowStyle m_innerGlow;
    BevelEmbossStyle m_bevelEmboss;
    StrokeStyle m_stroke;
    SatinStyle m_satin;
    ColorOverlayStyle m_colorOverlay;
    GradientOverlayStyle m_gradientOverlay;

    void renderDropShadow(PixelBuffer& buf, int w, int h) const;
    void renderInnerShadow(PixelBuffer& buf, int w, int h) const;
    void renderOuterGlow(PixelBuffer& buf, int w, int h) const;
    void renderInnerGlow(PixelBuffer& buf, int w, int h) const;
    void renderBevelEmboss(PixelBuffer& buf, int w, int h) const;
    void renderStroke(PixelBuffer& buf, int w, int h) const;
    void renderSatin(PixelBuffer& buf, int w, int h) const;
    void renderColorOverlay(PixelBuffer& buf, int w, int h) const;
    void renderGradientOverlay(PixelBuffer& buf, int w, int h) const;

    static std::vector<double> buildAlphaMask(const PixelBuffer& buf, int w, int h);
    static void boxBlurH(const std::vector<double>& src, std::vector<double>& dst, int w, int h, int radius);
    static void boxBlurV(const std::vector<double>& src, std::vector<double>& dst, int w, int h, int radius);
    static void gaussianBlur(const std::vector<double>& src, std::vector<double>& dst, int w, int h, double sigma);
    static std::vector<double> dilate(const std::vector<double>& mask, int w, int h, int radius);
    static bool isEdgePixel(const std::vector<double>& mask, int w, int h, int x, int y);
};

} // namespace FreeEffect
