#pragma once

#include "tool_interface.h"
#include <memory>

namespace FreeEffect {

class Camera;
class Composition;

class CameraToolBase : public Tool {
public:
    CameraToolBase() = default;
    virtual ~CameraToolBase() = default;

    void setCamera(std::shared_ptr<Camera> cam) { m_camera = cam; }
    void setComposition(std::shared_ptr<Composition> comp) { m_composition = comp; }

    void onMouseDown(double x, double y, int modifiers) override;
    void onMouseMove(double x, double y, int modifiers) override;
    void onMouseUp(double x, double y, int modifiers) override;
    ToolResult getResult() const override;
    void reset();

protected:
    virtual void applyDelta(double dx, double dy) = 0;

    std::shared_ptr<Camera> m_camera;
    std::shared_ptr<Composition> m_composition;
    Point2D m_startPos;
    bool m_dragging = false;
    double m_accumDx = 0;
    double m_accumDy = 0;
};

class OrbitCameraTool : public CameraToolBase {
public:
    ToolType getType() const override { return ToolType::OrbitCamera; }
    std::string getName() const override { return "Orbit Camera Tool"; }

protected:
    void applyDelta(double dx, double dy) override;
};

class TrackXYCameraTool : public CameraToolBase {
public:
    ToolType getType() const override { return ToolType::TrackXYCamera; }
    std::string getName() const override { return "Track XY Camera Tool"; }

protected:
    void applyDelta(double dx, double dy) override;
};

class TrackZCameraTool : public CameraToolBase {
public:
    ToolType getType() const override { return ToolType::TrackZCamera; }
    std::string getName() const override { return "Track Z Camera Tool"; }

protected:
    void applyDelta(double dx, double dy) override;
};

} // namespace FreeEffect
