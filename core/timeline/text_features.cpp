#include "text_features.h"
#include <cmath>
#include <algorithm>
#include <numeric>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace FreeEffect {

// ======================== Per-Character Styling ========================

TextStyle TextFeatures::getCharacterStyle(const std::string& text, int charIndex) {
    TextStyle style;
    if (charIndex < 0 || charIndex >= static_cast<int>(text.size())) return style;
    // In a full implementation, this would look up per-character overrides from a stored map
    // For now, return default style (applicable to all characters)
    return style;
}

void TextFeatures::setCharacterStyle(std::string& text, int charIndex, const TextStyle& style) {
    if (charIndex < 0 || charIndex >= static_cast<int>(text.size())) return;
    // Per-character styling is stored externally; this is a no-op placeholder
    // The actual implementation would store overrides in a map keyed by charIndex
    (void)text;
    (void)style;
}

// ======================== Variable Font ========================

std::vector<VariableFontAxis> TextFeatures::getVariableFontAxes(const std::string& fontFamily) {
    std::vector<VariableFontAxis> axes;
    // Common variable font axes (example for well-known fonts)
    if (fontFamily == "Inter" || fontFamily == "Roboto Flex" || fontFamily == "Open Sans") {
        axes.push_back({"wght", 400, 100, 900});
        axes.push_back({"wdth", 100, 75, 125});
        axes.push_back({"ital", 0, 0, 1});
        axes.push_back({"slnt", 0, -10, 0});
        axes.push_back({"opsz", 14, 8, 144});
    } else if (fontFamily == "Recursive") {
        axes.push_back({"wght", 300, 300, 1000});
        axes.push_back({"slnt", 0, -15, 0});
        axes.push_back({"CASL", 0, 0, 1});
        axes.push_back({"CRSV", 0.5, 0, 1});
        axes.push_back({"MONO", 0, 0, 1});
    } else if (fontFamily == "Fira Code" || fontFamily == "JetBrains Mono") {
        axes.push_back({"wght", 400, 300, 700});
    } else {
        // Generic axes for unknown fonts
        axes.push_back({"wght", 400, 100, 900});
    }
    return axes;
}

void TextFeatures::setVariableFontAxis(TextStyle& style, const std::string& axis, double value) {
    for (auto& va : style.variableFontAxes) {
        if (va.axisName == axis) {
            va.value = std::clamp(value, va.minValue, va.maxValue);
            return;
        }
    }
    // Add new axis if not present
    VariableFontAxis newAxis;
    newAxis.axisName = axis;
    newAxis.value = std::clamp(value, 0.0, 100.0);
    newAxis.minValue = 0;
    newAxis.maxValue = 100;
    style.variableFontAxes.push_back(newAxis);
}

// ======================== Text Measurement ========================

TextFeatures::TextMetrics TextFeatures::measureText(const std::string& text, const TextStyle& style) {
    TextMetrics metrics;
    if (text.empty()) return metrics;

    // Approximate character width using font metrics
    // Average character width ratio varies by font; use 0.6 as default
    double charWidthRatio = 0.6;
    if (style.fontFamily == "Courier New" || style.fontFamily == "Consolas" ||
        style.fontFamily == "Fira Code") {
        charWidthRatio = 0.6; // Monospace
    } else if (style.fontFamily == "Helvetica" || style.fontFamily == "Arial") {
        charWidthRatio = 0.55;
    } else if (style.fontFamily == "Times New Roman" || style.fontFamily == "Georgia") {
        charWidthRatio = 0.48;
    }

    double effectiveFontSize = style.fontSize * (style.hScale / 100.0);

    metrics.charWidths.resize(text.size());
    metrics.width = 0;
    int lineCount = 1;

    for (size_t i = 0; i < text.size(); i++) {
        char c = text[i];
        if (c == '\n') {
            lineCount++;
            metrics.charWidths[i] = 0;
            continue;
        }
        // Vary width by character
        double w = effectiveFontSize * charWidthRatio;
        if (c == ' ') w *= 0.3;
        else if (c == 'm' || c == 'M' || c == 'W') w *= 1.2;
        else if (c == 'i' || c == 'l' || c == 'j' || c == '1') w *= 0.5;
        else if (c == 'f' || c == 't') w *= 0.7;

        // Add tracking
        w += style.tracking * 0.01 * effectiveFontSize;

        metrics.charWidths[i] = w;
        metrics.width += w;
    }

    double effectiveLeading = (style.leading > 0) ? style.leading : effectiveFontSize * 1.2;
    metrics.height = lineCount * effectiveLeading;

    // Ascender ≈ 80% of font size, descender ≈ 20%
    metrics.ascender = effectiveFontSize * 0.8;
    metrics.descender = effectiveFontSize * 0.2;

    return metrics;
}

// ======================== Text on Path ========================

std::vector<Point2D> TextFeatures::getTextOnPath(const std::string& text, const TextStyle& style,
                                                  const std::vector<Point2D>& path,
                                                  const PathTextOptions& options) {
    std::vector<Point2D> positions;
    if (text.empty() || path.size() < 2) return positions;

    TextMetrics metrics = measureText(text, style);

    // Calculate total path length
    double totalPathLen = 0;
    for (size_t i = 1; i < path.size(); i++) {
        double dx = path[i].x - path[i - 1].x;
        double dy = path[i].y - path[i - 1].y;
        totalPathLen += std::sqrt(dx * dx + dy * dy);
    }

    double usableLength = totalPathLen - options.firstMargin - options.lastMargin;
    double totalTextWidth = metrics.width;

    // Start position along path
    double startDist = options.firstMargin;

    // Compute cumulative distances along path
    std::vector<double> cumDist(path.size(), 0);
    for (size_t i = 1; i < path.size(); i++) {
        double dx = path[i].x - path[i - 1].x;
        double dy = path[i].y - path[i - 1].y;
        cumDist[i] = cumDist[i - 1] + std::sqrt(dx * dx + dy * dy);
    }

    double currentDist = startDist;
    int pathSegIdx = 0;

    for (size_t ci = 0; ci < text.size(); ci++) {
        char c = text[ci];
        if (c == '\n') continue;

        // Find path segment for current distance
        while (pathSegIdx < static_cast<int>(path.size()) - 1 &&
               cumDist[pathSegIdx + 1] < currentDist) {
            pathSegIdx++;
        }

        if (pathSegIdx >= static_cast<int>(path.size()) - 1) break;

        double segLen = cumDist[pathSegIdx + 1] - cumDist[pathSegIdx];
        double localT = (segLen > 1e-12) ? (currentDist - cumDist[pathSegIdx]) / segLen : 0.0;
        localT = std::clamp(localT, 0.0, 1.0);

        // Position on path
        Point2D pos;
        pos.x = path[pathSegIdx].x + (path[pathSegIdx + 1].x - path[pathSegIdx].x) * localT;
        pos.y = path[pathSegIdx].y + (path[pathSegIdx + 1].y - path[pathSegIdx].y) * localT;

        positions.push_back(pos);

        // Advance by character width
        if (ci < metrics.charWidths.size()) {
            currentDist += metrics.charWidths[ci];
        }
    }

    return positions;
}

// ======================== Text to Shape Paths ========================

// Approximate glyph rendering using simple geometric primitives
static void appendCharRect(std::vector<BezierPoint>& path, double x, double y,
                           double w, double h, char c) {
    // Simplified glyph shapes for common characters
    // Each character gets a bounding rectangle approximation
    double left = x;
    double top = y;
    double right = x + w;
    double bottom = y + h;

    // Create rectangular path for character
    BezierPoint p0(left, top, left, top, right, top);
    BezierPoint p1(right, top, right, top, right, bottom);
    BezierPoint p2(right, bottom, right, bottom, left, bottom);
    BezierPoint p3(left, bottom, left, bottom, left, top);

    path.push_back(p0);
    path.push_back(p1);
    path.push_back(p2);
    path.push_back(p3);
}

static void appendGlyphOutline(std::vector<BezierPoint>& path, double x, double y,
                                 double w, double h, char c) {
    // Create simplified glyph outlines for common ASCII characters
    double cx = x + w * 0.5;
    double cy = y + h * 0.5;
    double r = w * 0.45;

    switch (c) {
        case 'o': case 'O': case '0': {
            // Approximate circle with 4 bezier arcs
            double k = 0.5522847498 * r;
            path.push_back({cx, cy - r, cx - k, cy - r, cx + k, cy - r});
            path.push_back({cx + r, cy, cx + r, cy - k, cx + r, cy + k});
            path.push_back({cx, cy + r, cx + k, cy + r, cx - k, cy + r});
            path.push_back({cx - r, cy, cx - r, cy + k, cx - r, cy - k});
            return;
        }
        case 'i': case 'I': case 'l': case '|': case '1': {
            // Vertical line
            path.push_back({cx, y, cx, y, cx, y + h});
            path.push_back({cx, y + h, cx, y + h, cx, y});
            return;
        }
        case '-': case '_': {
            double my = (c == '_') ? y + h : cy;
            path.push_back({x + w * 0.15, my, x + w * 0.15, my, x + w * 0.85, my});
            path.push_back({x + w * 0.85, my, x + w * 0.85, my, x + w * 0.15, my});
            return;
        }
        case '=': {
            double my1 = cy - h * 0.1;
            double my2 = cy + h * 0.1;
            path.push_back({x + w * 0.15, my1, x + w * 0.15, my1, x + w * 0.85, my1});
            path.push_back({x + w * 0.85, my1, x + w * 0.85, my1, x + w * 0.15, my1});
            path.push_back({x + w * 0.15, my2, x + w * 0.15, my2, x + w * 0.85, my2});
            path.push_back({x + w * 0.85, my2, x + w * 0.85, my2, x + w * 0.15, my2});
            return;
        }
        case '+': {
            double t = h * 0.15;
            // Horizontal bar
            path.push_back({x + w * 0.15, cy, x + w * 0.15, cy, x + w * 0.85, cy});
            path.push_back({x + w * 0.85, cy, x + w * 0.85, cy, x + w * 0.15, cy});
            // Vertical bar
            path.push_back({cx, y + t, cx, y + t, cx, y + h - t});
            path.push_back({cx, y + h - t, cx, y + h - t, cx, y + t});
            return;
        }
        default:
            break;
    }

    // Default: rectangular bounding box
    appendCharRect(path, x, y, w, h, c);
}

std::vector<BezierPoint> TextFeatures::textToShapePaths(const std::string& text, const TextStyle& style) {
    std::vector<BezierPoint> paths;
    if (text.empty()) return paths;

    TextMetrics metrics = measureText(text, style);
    double effectiveFontSize = style.fontSize * (style.vScale / 100.0);
    double effectiveLeading = (style.leading > 0) ? style.leading : effectiveFontSize * 1.2;

    double cursorX = 0;
    double cursorY = 0;

    for (size_t i = 0; i < text.size(); i++) {
        char c = text[i];
        if (c == '\n') {
            cursorX = 0;
            cursorY += effectiveLeading;
            continue;
        }

        double w = (i < metrics.charWidths.size()) ? metrics.charWidths[i] : effectiveFontSize * 0.6;
        double h = effectiveFontSize;

        // Baseline offset
        double y = cursorY + style.baselineShift * 0.01 * effectiveFontSize;

        size_t before = paths.size();
        appendGlyphOutline(paths, cursorX, y, w, h, c);
        if (paths.size() == before) {
            appendCharRect(paths, cursorX, y, w, h, c);
        }

        cursorX += w;
    }

    return paths;
}

// ======================== Text to Mask ========================

std::vector<BezierPoint> TextFeatures::textToMaskPaths(const std::string& text, const TextStyle& style) {
    // Text to mask is essentially the same as text to shape paths
    // but may use a different resolution or simplification
    std::vector<BezierPoint> paths = textToShapePaths(text, style);

    // Simplify mask paths slightly for better rendering
    if (paths.size() > 4) {
        // Simple deduplication of nearby points
        std::vector<BezierPoint> simplified;
        simplified.push_back(paths[0]);
        for (size_t i = 1; i < paths.size(); i++) {
            double dx = paths[i].position.x - paths[i - 1].position.x;
            double dy = paths[i].position.y - paths[i - 1].position.y;
            if (std::sqrt(dx * dx + dy * dy) > 0.5) {
                simplified.push_back(paths[i]);
            }
        }
        if (simplified.size() > 1) {
            simplified.push_back(simplified[0]); // Close path
        }
        return simplified;
    }

    return paths;
}

} // namespace FreeEffect
