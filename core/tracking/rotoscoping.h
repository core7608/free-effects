#pragma once
#include "../timeline/types.h"
#include "../timeline/mask.h"
#include <vector>
#include <map>
#include <memory>

namespace FreeEffect {

enum class RotoBrushMode { Foreground, Background };

struct RotoStroke {
    double time = 0;
    std::vector<Vec2> foregroundPoints;
    std::vector<Vec2> backgroundPoints;
};

struct RotoFrame {
    double time = 0;
    std::vector<MaskVertex> contour;
    bool propagated = false;
};

class RotoBrush {
public:
    void addStroke(double time, const RotoStroke& stroke);
    void propagateForward(double startTime, int frameCount);
    void propagateBackward(double startTime, int frameCount);
    
    std::vector<MaskVertex> getContour(double time) const;
    PixelBuffer generateMatte(int width, int height, double time) const;
    
    void setFeather(double pixels) { m_feather = pixels; }
    double getFeather() const { return m_feather; }
    
    void setBrushRadius(int pixels) { m_brushRadius = pixels; }

private:
    std::vector<RotoStroke> m_strokes;
    std::map<double, RotoFrame> m_frames;
    double m_feather = 1.0;
    int m_brushRadius = 10;
    
    RotoFrame propagateFrame(const RotoFrame& ref, const RotoStroke& stroke) const;
    std::vector<MaskVertex> computeContour(const RotoStroke& stroke, int width, int height) const;
    bool pointInPolygon(double px, double py, const std::vector<Vec2>& polygon) const;
    void traceContour(const std::vector<std::vector<bool>>& mask, int w, int h,
                      std::vector<MaskVertex>& contour) const;
};

} // namespace FreeEffect
