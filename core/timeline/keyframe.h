#pragma once

#include "keyframe_interpolation.h"
#include "types.h"
#include <variant>
#include <vector>

namespace FreeEffect {

class Keyframe {
public:
    Keyframe(double time, double value, InterpolationType interp = InterpolationType::Linear);
    
    double getTime() const { return m_time; }
    void setTime(double time) { m_time = time; }
    
    double getValue() const { return m_value; }
    void setValue(double value) { m_value = value; }
    
    InterpolationType getInterpolation() const { return m_interpolation; }
    void setInterpolation(InterpolationType interp) { m_interpolation = interp; }
    
    void setBezierHandles(double inHandle, double outHandle);
    double getInHandle() const { return m_inHandle; }
    double getOutHandle() const { return m_outHandle; }
    
    KeyframeInterpolation& getInterpolationData() { return m_interpolationData; }
    const KeyframeInterpolation& getInterpolationData() const { return m_interpolationData; }
    void setInterpolationData(const KeyframeInterpolation& data) { m_interpolationData = data; }

    double interpolate(const Keyframe& next, double time) const;

private:
    double m_time;
    double m_value;
    InterpolationType m_interpolation;
    double m_inHandle = 0.0;
    double m_outHandle = 0.0;
    KeyframeInterpolation m_interpolationData;
};

using KeyframeList = std::vector<Keyframe>;

} // namespace FreeEffect
