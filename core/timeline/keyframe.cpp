#include "keyframe.h"
#include <cmath>
#include <algorithm>

namespace FreeEffect {

Keyframe::Keyframe(double time, double value, InterpolationType interp)
    : m_time(time), m_value(value), m_interpolation(interp) {
}

void Keyframe::setBezierHandles(double inHandle, double outHandle) {
    m_inHandle = inHandle;
    m_outHandle = outHandle;
}

double Keyframe::interpolate(const Keyframe& next, double time) const {
    if (time <= m_time) return m_value;
    if (time >= next.getTime()) return next.getValue();
    
    double t = (time - m_time) / (next.getTime() - m_time);
    t = std::clamp(t, 0.0, 1.0);
    
    double result;
    switch (m_interpolation) {
        case InterpolationType::Linear:
            result = m_value + (next.getValue() - m_value) * t;
            break;
            
        case InterpolationType::EaseIn:
            result = m_value + (next.getValue() - m_value) * (t * t);
            break;
            
        case InterpolationType::EaseOut:
            result = m_value + (next.getValue() - m_value) * (1.0 - (1.0 - t) * (1.0 - t));
            break;
            
        case InterpolationType::EaseInOut:
            result = m_value + (next.getValue() - m_value) * 
                     (t < 0.5 ? 2.0 * t * t : 1.0 - std::pow(-2.0 * t + 2.0, 2) / 2.0);
            break;
            
        case InterpolationType::Bezier:
        {
            double t2 = t * t;
            double t3 = t2 * t;
            double mt = 1.0 - t;
            double mt2 = mt * mt;
            double mt3 = mt2 * mt;
            result = m_value * mt3 + 
                     (m_value + m_outHandle) * 3.0 * mt2 * t +
                     (next.getValue() + m_inHandle) * 3.0 * mt * t2 +
                     next.getValue() * t3;
            break;
        }
            
        case InterpolationType::Hold:
            result = m_value;
            break;
            
        default:
            result = m_value + (next.getValue() - m_value) * t;
            break;
    }
    
    return result;
}

} // namespace FreeEffect
