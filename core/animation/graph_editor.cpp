#include "graph_editor.h"
#include <cmath>
#include <algorithm>

namespace FreeEffect {

void GraphEditor::loadFromPropertyTrack(const PropertyTrack& track, const std::string& name) {
    m_curve.propertyName = name;
    m_curve.keyframes.clear();

    const auto& keyframes = track.getKeyframes();
    for (const auto& kf : keyframes) {
        GraphKeyframe gk;
        gk.time = kf.getTime();
        gk.value = kf.getValue();
        gk.interpolation = kf.getInterpolation();
        gk.incomingLength = kf.getInHandle();
        gk.outgoingLength = kf.getOutHandle();
        m_curve.keyframes.push_back(gk);
    }
}

PropertyTrack GraphEditor::toPropertyTrack() const {
    PropertyTrack track(m_curve.propertyName);

    for (const auto& gk : m_curve.keyframes) {
        Keyframe kf(gk.time, gk.value, gk.interpolation);
        kf.setBezierHandles(gk.incomingLength, gk.outgoingLength);
        track.addKeyframe(kf);
    }

    return track;
}

void GraphEditor::setKeyframeValue(int index, double value) {
    if (index >= 0 && index < static_cast<int>(m_curve.keyframes.size())) {
        m_curve.keyframes[index].value = value;
    }
}

void GraphEditor::setKeyframeSpeed(int index, double inSpeed, double outSpeed) {
    if (index >= 0 && index < static_cast<int>(m_curve.keyframes.size())) {
        m_curve.keyframes[index].speed = inSpeed;
        m_curve.keyframes[index].outgoingSpeed = outSpeed;
    }
}

void GraphEditor::setKeyframeInterpolation(int index, InterpolationType type) {
    if (index >= 0 && index < static_cast<int>(m_curve.keyframes.size())) {
        m_curve.keyframes[index].interpolation = type;
    }
}

void GraphEditor::setKeyframeTangent(int index, double angle, double length, bool incoming) {
    if (index >= 0 && index < static_cast<int>(m_curve.keyframes.size())) {
        if (incoming) {
            m_curve.keyframes[index].incomingAngle = angle;
            m_curve.keyframes[index].incomingLength = length;
        } else {
            m_curve.keyframes[index].outgoingAngle = angle;
            m_curve.keyframes[index].outgoingLength = length;
        }
    }
}

void GraphEditor::selectKeyframe(int index, bool selected) {
    if (index >= 0 && index < static_cast<int>(m_curve.keyframes.size())) {
        m_curve.keyframes[index].selected = selected;
    }
}

void GraphEditor::selectKeyframesInRange(double startTime, double endTime) {
    for (auto& gk : m_curve.keyframes) {
        gk.selected = (gk.time >= startTime && gk.time <= endTime);
    }
}

void GraphEditor::clearSelection() {
    for (auto& gk : m_curve.keyframes) {
        gk.selected = false;
    }
}

double GraphEditor::evaluate(double time) const {
    if (m_curve.keyframes.empty()) return 0.0;
    if (m_curve.keyframes.size() == 1) return m_curve.keyframes[0].value;

    // Before first keyframe
    if (time <= m_curve.keyframes.front().time) {
        return m_curve.keyframes.front().value;
    }
    // After last keyframe
    if (time >= m_curve.keyframes.back().time) {
        return m_curve.keyframes.back().value;
    }

    // Find surrounding keyframes
    for (size_t i = 0; i < m_curve.keyframes.size() - 1; ++i) {
        const auto& kf0 = m_curve.keyframes[i];
        const auto& kf1 = m_curve.keyframes[i + 1];

        if (time >= kf0.time && time <= kf1.time) {
            double dt = kf1.time - kf0.time;
            if (dt < 1e-12) return kf0.value;

            double t = (time - kf0.time) / dt;

            switch (kf0.interpolation) {
                case InterpolationType::Linear:
                    return kf0.value + (kf1.value - kf0.value) * t;

                case InterpolationType::Hold:
                    return kf0.value;

                case InterpolationType::EaseIn: {
                    double tt = t * t;
                    return kf0.value + (kf1.value - kf0.value) * tt;
                }

                case InterpolationType::EaseOut: {
                    double tt = 1.0 - (1.0 - t) * (1.0 - t);
                    return kf0.value + (kf1.value - kf0.value) * tt;
                }

                case InterpolationType::EaseInOut: {
                    double tt;
                    if (t < 0.5) {
                        tt = 2.0 * t * t;
                    } else {
                        tt = 1.0 - 2.0 * (1.0 - t) * (1.0 - t);
                    }
                    return kf0.value + (kf1.value - kf0.value) * tt;
                }

                default:
                    return kf0.value + (kf1.value - kf0.value) * t;
            }
        }
    }

    return m_curve.keyframes.back().value;
}

} // namespace FreeEffect
