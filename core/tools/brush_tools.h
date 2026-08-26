#pragma once

#include "tool_interface.h"
#include <vector>

namespace FreeEffect {

struct PaintStroke {
    struct StrokePoint {
        Point2D position;
        double pressure = 1.0;
        double timestamp = 0.0;
    };

    std::vector<StrokePoint> points;
    double brushSize = 10.0;
    double opacity = 1.0;
    double flow = 1.0;
    int colorR = 255, colorG = 255, colorB = 255;
    int frame = 0;
};

class BrushTool : public Tool {
public:
    BrushTool() = default;

    ToolType getType() const override { return ToolType::Brush; }
    std::string getName() const override { return "Brush Tool"; }
    std::string getIcon() const override { return ":/icons/tools/pen.svg"; }

    void onMouseDown(double x, double y, int modifiers) override;
    void onMouseMove(double x, double y, int modifiers) override;
    void onMouseUp(double x, double y, int modifiers) override;
    ToolResult getResult() const override;
    void reset();

    void setBrushSize(double size) { m_brushSize = size; }
    double getBrushSize() const { return m_brushSize; }
    void setOpacity(double opacity) { m_opacity = opacity; }
    void setFlow(double flow) { m_flow = flow; }
    void setColor(int r, int g, int b) { m_colorR = r; m_colorG = g; m_colorB = b; }
    const PaintStroke& getCurrentStroke() const { return m_currentStroke; }
    const std::vector<PaintStroke>& getAllStrokes() const { return m_strokes; }

protected:
    PaintStroke m_currentStroke;
    std::vector<PaintStroke> m_strokes;
    bool m_drawing = false;
    double m_brushSize = 10.0;
    double m_opacity = 1.0;
    double m_flow = 1.0;
    int m_colorR = 255, m_colorG = 255, m_colorB = 255;
    int m_currentFrame = 0;

    virtual void beginStroke(double x, double y, double pressure);
    virtual void addStrokePoint(double x, double y, double pressure);
    virtual void endStroke();
};

class CloneStampTool : public BrushTool {
public:
    CloneStampTool() = default;

    ToolType getType() const override { return ToolType::CloneStamp; }
    std::string getName() const override { return "Clone Stamp Tool"; }

    void onMouseDown(double x, double y, int modifiers) override;
    void onMouseMove(double x, double y, int modifiers) override;

    void setSourcePoint(double x, double y) { m_sourcePoint = {x, y}; }

private:
    Point2D m_sourcePoint;
    Point2D m_cloneOffset;
};

class EraserTool : public BrushTool {
public:
    EraserTool() = default;

    ToolType getType() const override { return ToolType::Eraser; }
    std::string getName() const override { return "Eraser Tool"; }

    void onMouseDown(double x, double y, int modifiers) override;
    void onMouseMove(double x, double y, int modifiers) override;
    void onMouseUp(double x, double y, int modifiers) override;
};

} // namespace FreeEffect
