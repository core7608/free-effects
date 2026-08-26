#pragma once

#include "tool_interface.h"

namespace FreeEffect {

class Composition;

class HandTool : public Tool {
public:
    HandTool() = default;

    ToolType getType() const override { return ToolType::Hand; }
    std::string getName() const override { return "Hand Tool"; }
    std::string getIcon() const override { return ":/icons/tools/hand.svg"; }

    void onMouseDown(double x, double y, int modifiers) override;
    void onMouseMove(double x, double y, int modifiers) override;
    void onMouseUp(double x, double y, int modifiers) override;
    ToolResult getResult() const override;
    void reset();

private:
    Point2D m_startPos;
    Point2D m_delta;
    bool m_dragging = false;
};

class ZoomTool : public Tool {
public:
    ZoomTool() = default;

    ToolType getType() const override { return ToolType::Zoom; }
    std::string getName() const override { return "Zoom Tool"; }
    std::string getIcon() const override { return ":/icons/tools/zoom.svg"; }

    void onMouseDown(double x, double y, int modifiers) override;
    void onMouseUp(double x, double y, int modifiers) override;
    ToolResult getResult() const override;
    void reset();

private:
    bool m_zoomIn = true;
    Point2D m_clickPos;
};

class RotationTool : public Tool {
public:
    RotationTool() = default;

    ToolType getType() const override { return ToolType::Rotation; }
    std::string getName() const override { return "Rotation Tool"; }
    std::string getIcon() const override { return ":/icons/tools/rotation.svg"; }

    void onMouseDown(double x, double y, int modifiers) override;
    void onMouseMove(double x, double y, int modifiers) override;
    void onMouseUp(double x, double y, int modifiers) override;
    ToolResult getResult() const override;
    void reset();

    void setAnchorPoint(double x, double y) { m_anchorPoint = {x, y}; }

private:
    Point2D m_anchorPoint;
    Point2D m_startPos;
    double m_rotationDelta = 0.0;
    bool m_dragging = false;
};

class PanBehindTool : public Tool {
public:
    PanBehindTool() = default;

    ToolType getType() const override { return ToolType::PanBehind; }
    std::string getName() const override { return "Pan Behind Tool"; }
    std::string getIcon() const override { return ":/icons/tools/anchor.svg"; }

    void onMouseDown(double x, double y, int modifiers) override;
    void onMouseMove(double x, double y, int modifiers) override;
    void onMouseUp(double x, double y, int modifiers) override;
    ToolResult getResult() const override;
    void reset();

private:
    Point2D m_startPos;
    Point2D m_delta;
    bool m_dragging = false;
};

} // namespace FreeEffect
