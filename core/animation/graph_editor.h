#pragma once
#include "../timeline/property_track.h"
#include "../timeline/keyframe.h"
#include <vector>

namespace FreeEffect {

enum class GraphType { Value, Speed };

struct GraphKeyframe {
    double time = 0;
    double value = 0;
    double speed = 0;               // Incoming speed
    double outgoingSpeed = 0;       // Outgoing speed
    double incomingAngle = 0;       // Bezier handle angle
    double incomingLength = 0;      // Bezier handle length
    double outgoingAngle = 0;
    double outgoingLength = 0;
    bool selected = false;
    InterpolationType interpolation = InterpolationType::Linear;
};

struct GraphCurve {
    std::string propertyName;
    std::vector<GraphKeyframe> keyframes;
};

class GraphEditor {
public:
    void setGraphType(GraphType type) { m_graphType = type; }
    GraphType getGraphType() const { return m_graphType; }
    
    void loadFromPropertyTrack(const PropertyTrack& track, const std::string& name);
    PropertyTrack toPropertyTrack() const;
    
    const GraphCurve& getCurrentCurve() const { return m_curve; }
    GraphCurve& getCurrentCurve() { return m_curve; }
    
    void setKeyframeValue(int index, double value);
    void setKeyframeSpeed(int index, double inSpeed, double outSpeed);
    void setKeyframeInterpolation(int index, InterpolationType type);
    void setKeyframeTangent(int index, double angle, double length, bool incoming);
    
    void selectKeyframe(int index, bool selected);
    void selectKeyframesInRange(double startTime, double endTime);
    void clearSelection();
    
    // Compute interpolated value at time based on current curve
    double evaluate(double time) const;

private:
    GraphType m_graphType = GraphType::Value;
    GraphCurve m_curve;
};

} // namespace FreeEffect
