#include "keyframe_assistant.h"
#include <algorithm>
#include <cmath>
#include <numeric>

namespace FreeEffect {

void KeyframeAssistant::easyEase(std::vector<Keyframe>& keyframes,
                                 const std::vector<int>& selectedIndices) {
    for (int idx : selectedIndices) {
        if (idx < 0 || idx >= static_cast<int>(keyframes.size())) continue;

        Keyframe& kf = keyframes[idx];
        kf.setInterpolation(InterpolationType::Bezier);

        KeyframeInterpolation interp;
        interp.temporalType = InterpolationType::Bezier;

        // Easy Ease: 33.3% influence on both in and out handles
        interp.temporalIn.inX = 2.0 / 3.0;
        interp.temporalIn.inY = 1.0 - 0.333;
        interp.temporalOut.outX = 1.0 / 3.0;
        interp.temporalOut.outY = 0.333;

        interp.continuous = true;
        kf.setInterpolationData(interp);

        // Also set the legacy handles
        kf.setBezierHandles(0.333, 0.333);
    }
}

void KeyframeAssistant::easyEaseIn(std::vector<Keyframe>& keyframes,
                                   const std::vector<int>& selectedIndices) {
    for (int idx : selectedIndices) {
        if (idx < 0 || idx >= static_cast<int>(keyframes.size())) continue;

        Keyframe& kf = keyframes[idx];
        kf.setInterpolation(InterpolationType::Bezier);

        KeyframeInterpolation interp;
        interp.temporalType = InterpolationType::EaseIn;

        // Ease In: bezier on incoming, linear on outgoing
        interp.temporalIn.inX = 2.0 / 3.0;
        interp.temporalIn.inY = 1.0 - 0.333;
        interp.temporalOut.outX = 1.0;
        interp.temporalOut.outY = 1.0;

        interp.continuous = false;
        kf.setInterpolationData(interp);
        kf.setBezierHandles(0.333, 0.0);
    }
}

void KeyframeAssistant::easyEaseOut(std::vector<Keyframe>& keyframes,
                                    const std::vector<int>& selectedIndices) {
    for (int idx : selectedIndices) {
        if (idx < 0 || idx >= static_cast<int>(keyframes.size())) continue;

        Keyframe& kf = keyframes[idx];
        kf.setInterpolation(InterpolationType::Bezier);

        KeyframeInterpolation interp;
        interp.temporalType = InterpolationType::EaseOut;

        // Ease Out: linear on incoming, bezier on outgoing
        interp.temporalIn.inX = 0.0;
        interp.temporalIn.inY = 0.0;
        interp.temporalOut.outX = 1.0 / 3.0;
        interp.temporalOut.outY = 0.333;

        interp.continuous = false;
        kf.setInterpolationData(interp);
        kf.setBezierHandles(0.0, 0.333);
    }
}

std::vector<Keyframe> KeyframeAssistant::exponentialScale(
    double startValue, double endValue,
    double startTime, double endTime, int numKeyframes) {

    std::vector<Keyframe> result;
    if (numKeyframes < 2) numKeyframes = 2;
    if (endTime <= startTime) return result;

    // Exponential scale: values follow an exponential curve from startValue to endValue
    // log(startValue) to log(endValue) linearly interpolated, then exp()
    double logStart = std::log(std::max(startValue, 1e-10));
    double logEnd = std::log(std::max(endValue, 1e-10));

    for (int i = 0; i < numKeyframes; ++i) {
        double t = static_cast<double>(i) / (numKeyframes - 1);
        double time = startTime + t * (endTime - startTime);
        double logVal = logStart + t * (logEnd - logStart);
        double value = std::exp(logVal);

        Keyframe kf(time, value, InterpolationType::Linear);
        result.push_back(kf);
    }

    return result;
}

void KeyframeAssistant::timeReverseKeyframes(std::vector<Keyframe>& keyframes,
                                             double duration) {
    if (keyframes.size() < 2) return;

    // Find the midpoint time
    double minTime = keyframes.front().getTime();
    double maxTime = keyframes.back().getTime();
    double midTime = (minTime + maxTime) / 2.0;

    // Mirror each keyframe around the midpoint, preserving relative values
    for (auto& kf : keyframes) {
        double oldTime = kf.getTime();
        double relativeToMid = oldTime - midTime;
        double newTime = midTime - relativeToMid;
        kf.setTime(newTime);
    }

    // Sort by time after reversal
    std::sort(keyframes.begin(), keyframes.end(),
              [](const Keyframe& a, const Keyframe& b) {
                  return a.getTime() < b.getTime();
              });

    // Clamp times within [0, duration]
    if (duration > 0) {
        for (auto& kf : keyframes) {
            double t = kf.getTime();
            if (t < 0) kf.setTime(0);
            if (t > duration) kf.setTime(duration);
        }
    }
}

void KeyframeAssistant::toggleHold(std::vector<Keyframe>& keyframes,
                                   const std::vector<int>& selectedIndices) {
    for (int idx : selectedIndices) {
        if (idx < 0 || idx >= static_cast<int>(keyframes.size())) continue;

        Keyframe& kf = keyframes[idx];
        if (kf.getInterpolation() == InterpolationType::Hold) {
            // Toggle back to linear
            kf.setInterpolation(InterpolationType::Linear);
            KeyframeInterpolation interp;
            interp.temporalType = InterpolationType::Linear;
            kf.setInterpolationData(interp);
        } else {
            // Set to hold
            kf.setInterpolation(InterpolationType::Hold);
            KeyframeInterpolation interp;
            interp.temporalType = InterpolationType::Hold;
            kf.setInterpolationData(interp);
        }
    }
}

Keyframe KeyframeAssistant::addKeyframeAtTime(const PropertyTrack& track, double time) {
    double value = track.getValueAtTime(time);
    return Keyframe(time, value, InterpolationType::Linear);
}

int KeyframeAssistant::findNextKeyframe(const std::vector<Keyframe>& keyframes, double currentTime) {
    for (int i = 0; i < static_cast<int>(keyframes.size()); ++i) {
        if (keyframes[i].getTime() > currentTime + 1e-9) {
            return i;
        }
    }
    return -1;
}

int KeyframeAssistant::findPrevKeyframe(const std::vector<Keyframe>& keyframes, double currentTime) {
    for (int i = static_cast<int>(keyframes.size()) - 1; i >= 0; --i) {
        if (keyframes[i].getTime() < currentTime - 1e-9) {
            return i;
        }
    }
    return -1;
}

std::vector<KeyframeVelocity> KeyframeAssistant::computeVelocities(
    const std::vector<Keyframe>& keyframes) {

    std::vector<KeyframeVelocity> velocities;
    velocities.reserve(keyframes.size());

    for (size_t i = 0; i < keyframes.size(); ++i) {
        KeyframeVelocity vel;
        vel.time = keyframes[i].getTime();

        // Outgoing velocity (slope to next keyframe)
        if (i + 1 < keyframes.size()) {
            double dt = keyframes[i + 1].getTime() - keyframes[i].getTime();
            if (dt > 1e-9) {
                vel.outVelocity = (keyframes[i + 1].getValue() - keyframes[i].getValue()) / dt;
            }
        } else {
            vel.outVelocity = 0;
        }

        // Incoming velocity (slope from previous keyframe)
        if (i > 0) {
            double dt = keyframes[i].getTime() - keyframes[i - 1].getTime();
            if (dt > 1e-9) {
                vel.inVelocity = (keyframes[i].getValue() - keyframes[i - 1].getValue()) / dt;
            }
        } else {
            vel.inVelocity = 0;
        }

        // Influence based on interpolation type
        InterpolationType interp = keyframes[i].getInterpolation();
        switch (interp) {
            case InterpolationType::Linear:
                vel.inType = 0;
                vel.outType = 0;
                vel.inInfluence = 0;
                vel.outInfluence = 0;
                break;
            case InterpolationType::Bezier:
            case InterpolationType::ContinuousBezier:
            case InterpolationType::AutoBezier:
                vel.inType = 1;
                vel.outType = 1;
                vel.inInfluence = 33.3;
                vel.outInfluence = 33.3;
                break;
            case InterpolationType::EaseIn:
                vel.inType = 2;
                vel.outType = 0;
                vel.inInfluence = 33.3;
                vel.outInfluence = 0;
                break;
            case InterpolationType::EaseOut:
                vel.inType = 0;
                vel.outType = 2;
                vel.inInfluence = 0;
                vel.outInfluence = 33.3;
                break;
            case InterpolationType::EaseInOut:
                vel.inType = 2;
                vel.outType = 2;
                vel.inInfluence = 33.3;
                vel.outInfluence = 33.3;
                break;
            case InterpolationType::Hold:
                vel.inType = 0;
                vel.outType = 0;
                vel.inInfluence = 0;
                vel.outInfluence = 0;
                break;
            default:
                vel.inType = 0;
                vel.outType = 0;
                vel.inInfluence = 0;
                vel.outInfluence = 0;
                break;
        }

        velocities.push_back(vel);
    }

    return velocities;
}

void KeyframeAssistant::roveVertices(std::vector<Keyframe>& keyframes,
                                     const std::vector<int>& selectedIndices) {
    if (selectedIndices.size() < 3) return;

    // Sort selected indices
    std::vector<int> sorted = selectedIndices;
    std::sort(sorted.begin(), sorted.end());

    // Get the first and last selected keyframes (anchors)
    int firstIdx = sorted.front();
    int lastIdx = sorted.back();
    if (firstIdx < 0 || lastIdx >= static_cast<int>(keyframes.size())) return;
    if (firstIdx >= lastIdx) return;

    double firstTime = keyframes[firstIdx].getTime();
    double lastTime = keyframes[lastIdx].getTime();
    double totalDuration = lastTime - firstTime;
    if (totalDuration <= 0) return;

    // Calculate cumulative "distance" between consecutive selected keyframes
    // using value differences as the metric
    std::vector<double> distances;
    distances.push_back(0.0);
    for (size_t i = 1; i < sorted.size(); ++i) {
        int prevIdx = sorted[i - 1];
        int currIdx = sorted[i];
        double valueDiff = std::abs(keyframes[currIdx].getValue() - keyframes[prevIdx].getValue());
        double prev = distances.back();
        // Use Euclidean distance in time-value space
        double timeDiff = keyframes[currIdx].getTime() - keyframes[prevIdx].getTime();
        double dist = std::sqrt(timeDiff * timeDiff + valueDiff * valueDiff);
        distances.push_back(prev + dist);
    }

    double totalDistance = distances.back();
    if (totalDistance < 1e-9) return;

    // Redistribute timing proportionally
    for (size_t i = 1; i < sorted.size() - 1; ++i) {
        int idx = sorted[i];
        double ratio = distances[i] / totalDistance;
        double newTime = firstTime + ratio * totalDuration;
        keyframes[idx].setTime(newTime);
    }
}

void KeyframeAssistant::convertToLinear(std::vector<Keyframe>& keyframes,
                                        const std::vector<int>& selectedIndices) {
    for (int idx : selectedIndices) {
        if (idx < 0 || idx >= static_cast<int>(keyframes.size())) continue;
        keyframes[idx].setInterpolation(InterpolationType::Linear);
        KeyframeInterpolation interp;
        interp.temporalType = InterpolationType::Linear;
        keyframes[idx].setInterpolationData(interp);
    }
}

void KeyframeAssistant::convertToBezier(std::vector<Keyframe>& keyframes,
                                        const std::vector<int>& selectedIndices) {
    for (int idx : selectedIndices) {
        if (idx < 0 || idx >= static_cast<int>(keyframes.size())) continue;

        keyframes[idx].setInterpolation(InterpolationType::Bezier);

        KeyframeInterpolation interp;
        interp.temporalType = InterpolationType::Bezier;

        // Set default bezier handles
        interp.temporalIn.inX = 2.0 / 3.0;
        interp.temporalIn.inY = 0.0;
        interp.temporalOut.outX = 1.0 / 3.0;
        interp.temporalOut.outY = 1.0;

        keyframes[idx].setInterpolationData(interp);
        keyframes[idx].setBezierHandles(0.0, 1.0);
    }
}

void KeyframeAssistant::convertToHold(std::vector<Keyframe>& keyframes,
                                      const std::vector<int>& selectedIndices) {
    for (int idx : selectedIndices) {
        if (idx < 0 || idx >= static_cast<int>(keyframes.size())) continue;
        keyframes[idx].setInterpolation(InterpolationType::Hold);
        KeyframeInterpolation interp;
        interp.temporalType = InterpolationType::Hold;
        keyframes[idx].setInterpolationData(interp);
    }
}

} // namespace FreeEffect
