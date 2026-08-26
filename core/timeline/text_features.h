#pragma once

#include "types.h"
#include "shape_operations.h"
#include <string>
#include <vector>
#include <functional>

namespace FreeEffect {

struct PathTextOptions {
    bool enabled = false;
    std::vector<Point2D> pathPoints;
    bool reversePath = false;
    double firstMargin = 0;
    double lastMargin = 0;
    bool perpToPath = true;
};

struct PerCharacter3D {
    bool enabled = false;
    Vec3 position{0, 0, 0};
    Vec3 rotation{0, 0, 0};
    Vec3 scale{1, 1, 1};
    double opacity = 100;
    Color fillColor{1, 1, 1, 1};
    Color strokeColor{0, 0, 0, 0};
};

struct VariableFontAxis {
    std::string axisName;
    double value = 0;
    double minValue = 0;
    double maxValue = 100;
};

struct TextStyle {
    std::string fontFamily = "Arial";
    std::string fontStyle = "Regular";
    double fontSize = 72;
    double leading = 86.4;
    double tracking = 0;
    double kerning = 0;
    double hScale = 100;
    double vScale = 100;
    double baselineShift = 0;
    bool fauxBold = false;
    bool fauxItalic = false;
    int justification = 0; // 0=Left, 1=Center, 2=Right, 3=LastLeft, 4=LastCenter, 5=LastRight
    Color fillColor{1, 1, 1, 1};
    double fillOpacity = 100;
    Color strokeColor{0, 0, 0, 0};
    double strokeWidth = 0;
    double strokeOpacity = 100;
    int paintOrder = 0; // 0=Fill, 1=Stroke, 2=Fill then Stroke
    int strokeOverFill = 1;
    std::vector<VariableFontAxis> variableFontAxes;
};

struct ParagraphStyle {
    int justification = 0;
    double indent = 0;
    double leftMargin = 0;
    double rightMargin = 0;
    double spaceBefore = 0;
    double spaceAfter = 0;
    bool autoHyphenate = true;
    int hyphenateWordsBefore = 2;
    int hyphenateWordsAfter = 2;
    int hyphenateLimit = 0;
    int composer = 0; // 0=Single-line, 1=Multi-line
};

class TextFeatures {
public:
    // Per-character styling
    static TextStyle getCharacterStyle(const std::string& text, int charIndex);
    static void setCharacterStyle(std::string& text, int charIndex, const TextStyle& style);

    // Variable font support
    static std::vector<VariableFontAxis> getVariableFontAxes(const std::string& fontFamily);
    static void setVariableFontAxis(TextStyle& style, const std::string& axis, double value);

    // Path text
    static std::vector<Point2D> getTextOnPath(const std::string& text, const TextStyle& style,
                                               const std::vector<Point2D>& path, const PathTextOptions& options);

    // Text measurement
    struct TextMetrics {
        double width = 0;
        double height = 0;
        double ascender = 0;
        double descender = 0;
        std::vector<double> charWidths;
    };
    static TextMetrics measureText(const std::string& text, const TextStyle& style);

    // Auto-trace text to paths
    static std::vector<BezierPoint> textToShapePaths(const std::string& text, const TextStyle& style);

    // Text to mask
    static std::vector<BezierPoint> textToMaskPaths(const std::string& text, const TextStyle& style);
};

} // namespace FreeEffect
