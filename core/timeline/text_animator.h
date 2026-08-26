#pragma once

#include "property_track.h"
#include "types.h"
#include <string>
#include <vector>

namespace FreeEffect {

enum class AnimatorProperty {
    Position,
    Scale,
    Rotation,
    Opacity,
    AnchorPoint,
    FillColor,
    StrokeColor,
    Tracking,
    LineAnchor
};

enum class SelectorType {
    Range,
    Wiggle,
    Expression,
    Index
};

struct TextAnimator {
    std::string name;
    AnimatorProperty property = AnimatorProperty::Position;
    PropertyTrack value;
    SelectorType selectorType = SelectorType::Range;
    int rangeStart = 0;
    int rangeEnd = -1;
    bool basedOnCharacters = true;

    TextAnimator()
        : value("Animator Value") {
    }

    explicit TextAnimator(const std::string& animatorName)
        : name(animatorName)
        , value(animatorName + " Value") {
    }

    int getEffectiveRangeEnd(int totalChars) const {
        return (rangeEnd < 0) ? totalChars : std::min(rangeEnd, totalChars);
    }
};

class TextAnimationData {
public:
    TextAnimationData() = default;
    explicit TextAnimationData(const std::string& text) : m_text(text) {}

    const std::string& getText() const { return m_text; }
    void setText(const std::string& text) { m_text = text; }

    const std::string& getFontFamily() const { return m_fontFamily; }
    void setFontFamily(const std::string& font) { m_fontFamily = font; }

    double getFontSize() const { return m_fontSize; }
    void setFontSize(double size) { m_fontSize = size; }

    bool isBold() const { return m_bold; }
    void setBold(bool bold) { m_bold = bold; }

    bool isItalic() const { return m_italic; }
    void setItalic(bool italic) { m_italic = italic; }

    Color getFillColor() const { return m_fillColor; }
    void setFillColor(const Color& color) { m_fillColor = color; }

    Color getStrokeColor() const { return m_strokeColor; }
    void setStrokeColor(const Color& color) { m_strokeColor = color; }

    double getStrokeWidth() const { return m_strokeWidth; }
    void setStrokeWidth(double width) { m_strokeWidth = width; }

    double getTracking() const { return m_tracking; }
    void setTracking(double tracking) { m_tracking = tracking; }

    double getLeading() const { return m_leading; }
    void setLeading(double leading) { m_leading = leading; }

    int getJustification() const { return m_justification; }
    void setJustification(int just) { m_justification = just; }

    std::vector<TextAnimator>& getAnimators() { return m_animators; }
    const std::vector<TextAnimator>& getAnimators() const { return m_animators; }

    void addAnimator(const TextAnimator& animator) { m_animators.push_back(animator); }
    void removeAnimator(int index) {
        if (index >= 0 && index < static_cast<int>(m_animators.size())) {
            m_animators.erase(m_animators.begin() + index);
        }
    }

    int getCharacterCount() const { return static_cast<int>(m_text.size()); }

    double getLineWidth() const {
        return m_text.size() * (m_fontSize * 0.6 + m_tracking);
    }

    double getTotalHeight() const {
        int lineCount = 1;
        for (char c : m_text) {
            if (c == '\n') lineCount++;
        }
        double effectiveLeading = (m_leading > 0) ? m_leading : m_fontSize * 1.2;
        return lineCount * effectiveLeading;
    }

private:
    std::string m_text;
    std::string m_fontFamily = "Arial";
    double m_fontSize = 72.0;
    bool m_bold = false;
    bool m_italic = false;
    Color m_fillColor{1.0, 1.0, 1.0, 1.0};
    Color m_strokeColor{0.0, 0.0, 0.0, 0.0};
    double m_strokeWidth = 0.0;
    double m_tracking = 0.0;
    double m_leading = 0.0;
    int m_justification = 0;
    std::vector<TextAnimator> m_animators;
};

} // namespace FreeEffect
