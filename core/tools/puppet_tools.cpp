#include "puppet_tools.h"
#include <cmath>
#include <algorithm>

namespace FreeEffect {

void PuppetToolBase::onMouseDown(double x, double y, int modifiers) {
    double bestDist = 10.0;
    m_dragIndex = -1;

    for (int i = 0; i < static_cast<int>(m_pins.size()); i++) {
        if (m_pins[i].locked) continue;
        double dx = x - m_pins[i].position.x;
        double dy = y - m_pins[i].position.y;
        double dist = std::sqrt(dx * dx + dy * dy);
        if (dist < bestDist) {
            bestDist = dist;
            m_dragIndex = i;
        }
    }

    if (m_dragIndex >= 0) {
        m_draggingPin = true;
    } else {
        PuppetPin pin;
        pin.position = {x, y};
        pin.influence = 1.0;
        pin.radius = 50.0;
        m_pins.push_back(pin);
        m_dragIndex = static_cast<int>(m_pins.size()) - 1;
        m_draggingPin = true;
    }
}

void PuppetToolBase::onMouseMove(double x, double y, int modifiers) {
    if (m_draggingPin && m_dragIndex >= 0 &&
        m_dragIndex < static_cast<int>(m_pins.size())) {
        m_pins[m_dragIndex].position = {x, y};
    }
}

void PuppetToolBase::onMouseUp(double x, double y, int modifiers) {
    m_draggingPin = false;
    m_dragIndex = -1;
}

ToolResult PuppetToolBase::getResult() const {
    ToolResult result;
    result.consumed = m_draggingPin || !m_pins.empty();
    for (const auto& pin : m_pins) {
        result.points.push_back(pin.position);
    }
    return result;
}

void PuppetToolBase::reset() {
    m_pins.clear();
    m_draggingPin = false;
    m_dragIndex = -1;
}

static double distanceWeightedDeform(
    const Point2D& original, const std::vector<PuppetPin>& pins,
    double power) {

    if (pins.empty()) return 0.0;

    double totalWeight = 0.0;
    double totalDisplacement = 0.0;

    for (const auto& pin : pins) {
        double dx = original.x - pin.position.x;
        double dy = original.y - pin.position.y;
        double dist = std::sqrt(dx * dx + dy * dy) + 1.0;
        double weight = pin.influence / std::pow(dist, power);
        totalWeight += weight;
        totalDisplacement += weight * dist;
    }

    if (totalWeight < 1e-12) return 0.0;
    return totalDisplacement / totalWeight;
}

double PuppetPinTool::computeDeformation(
    const Point2D& original, const std::vector<PuppetPin>& pins) const {
    return distanceWeightedDeform(original, pins, 2.0);
}

double PuppetStretchTool::computeDeformation(
    const Point2D& original, const std::vector<PuppetPin>& pins) const {
    return distanceWeightedDeform(original, pins, 1.5);
}

double PuppetBendTool::computeDeformation(
    const Point2D& original, const std::vector<PuppetPin>& pins) const {
    return distanceWeightedDeform(original, pins, 3.0);
}

double PuppetStarchTool::computeDeformation(
    const Point2D& original, const std::vector<PuppetPin>& pins) const {
    return distanceWeightedDeform(original, pins, 4.0);
}

} // namespace FreeEffect
