#pragma once

#include "tool_interface.h"
#include <vector>

namespace FreeEffect {

struct PuppetPin {
    Point2D position;
    double influence = 1.0;
    double radius = 50.0;
    bool locked = false;
};

class PuppetToolBase : public Tool {
public:
    PuppetToolBase() = default;
    virtual ~PuppetToolBase() = default;

    void onMouseDown(double x, double y, int modifiers) override;
    void onMouseMove(double x, double y, int modifiers) override;
    void onMouseUp(double x, double y, int modifiers) override;
    ToolResult getResult() const override;
    void reset();

    const std::vector<PuppetPin>& getPins() const { return m_pins; }
    void setMeshBounds(double x, double y, double w, double h) {
        m_meshX = x; m_meshY = y; m_meshW = w; m_meshH = h;
    }

protected:
    virtual double computeDeformation(
        const Point2D& original, const std::vector<PuppetPin>& pins) const = 0;

    std::vector<PuppetPin> m_pins;
    bool m_draggingPin = false;
    int m_dragIndex = -1;
    double m_meshX = 0, m_meshY = 0, m_meshW = 100, m_meshH = 100;
};

class PuppetPinTool : public PuppetToolBase {
public:
    ToolType getType() const override { return ToolType::PuppetPin; }
    std::string getName() const override { return "Puppet Pin Tool"; }

protected:
    double computeDeformation(
        const Point2D& original, const std::vector<PuppetPin>& pins) const override;
};

class PuppetStretchTool : public PuppetToolBase {
public:
    ToolType getType() const override { return ToolType::PuppetStretch; }
    std::string getName() const override { return "Puppet Stretch Tool"; }

protected:
    double computeDeformation(
        const Point2D& original, const std::vector<PuppetPin>& pins) const override;
};

class PuppetBendTool : public PuppetToolBase {
public:
    ToolType getType() const override { return ToolType::PuppetBend; }
    std::string getName() const override { return "Puppet Bend Tool"; }

protected:
    double computeDeformation(
        const Point2D& original, const std::vector<PuppetPin>& pins) const override;
};

class PuppetStarchTool : public PuppetToolBase {
public:
    ToolType getType() const override { return ToolType::PuppetStarch; }
    std::string getName() const override { return "Puppet Starch Tool"; }

protected:
    double computeDeformation(
        const Point2D& original, const std::vector<PuppetPin>& pins) const override;
};

} // namespace FreeEffect
