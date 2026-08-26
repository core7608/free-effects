#include "keyframe_operations.h"
#include <cmath>
#include <algorithm>
#include <numeric>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace FreeEffect {

// ======================== Audio to Keyframes ========================

std::vector<Keyframe> KeyframeOperations::audioToKeyframes(const std::vector<float>& audioSamples,
                                                           int sampleRate, double threshold) {
    std::vector<Keyframe> keyframes;
    if (audioSamples.empty() || sampleRate <= 0) return keyframes;

    // Downsample: find peaks in blocks
    int blockSize = sampleRate / 30; // ~30 fps equivalent
    if (blockSize < 1) blockSize = 1;

    for (int i = 0; i < static_cast<int>(audioSamples.size()); i += blockSize) {
        int end = std::min(i + blockSize, static_cast<int>(audioSamples.size()));
        float maxAmp = 0.0f;
        for (int j = i; j < end; j++) {
            float amp = std::abs(audioSamples[j]);
            if (amp > maxAmp) maxAmp = amp;
        }

        double time = static_cast<double>(i) / static_cast<double>(sampleRate);
        double value = static_cast<double>(maxAmp) * 100.0; // Scale to 0-100

        // Only add keyframe if above threshold
        if (value >= threshold || keyframes.empty()) {
            keyframes.emplace_back(time, value, InterpolationType::Linear);
        }
    }

    return keyframes;
}

// ======================== Expression to Keyframes ========================

std::vector<Keyframe> KeyframeOperations::expressionToKeyframes(const PropertyTrack& track,
                                                                double startTime, double endTime,
                                                                double sampleRate) {
    std::vector<Keyframe> keyframes;
    if (sampleRate <= 0 || startTime >= endTime) return keyframes;

    double step = 1.0 / sampleRate;
    for (double t = startTime; t <= endTime; t += step) {
        double value = track.getValueAtTime(t);
        keyframes.emplace_back(t, value, InterpolationType::Linear);
    }

    return keyframes;
}

// ======================== Sequence Layers ========================

void KeyframeOperations::sequenceLayers(std::vector<std::shared_ptr<Layer>>& layers,
                                         double gap, int order, bool overlap) {
    if (layers.empty()) return;

    // Sort layers based on order
    if (order == 0) {
        // By name
        std::sort(layers.begin(), layers.end(),
                  [](const auto& a, const auto& b) { return a->getName() < b->getName(); });
    } else if (order == 1) {
        // By index (keep current order)
    } else if (order == 2) {
        // Reverse
        std::reverse(layers.begin(), layers.end());
    }

    double currentTime = 0.0;
    for (auto& layer : layers) {
        double duration = layer->getDuration();
        layer->setStartTime(currentTime);

        if (overlap) {
            // Overlap: next layer starts at current + duration - overlap amount
            currentTime += duration * 0.5 + gap;
        } else {
            currentTime += duration + gap;
        }
    }
}

// ======================== Apply Keyframe Velocity ========================

void KeyframeOperations::applyKeyframeVelocity(std::vector<Keyframe>& keyframes,
                                                int index, double inInfluence, double outInfluence,
                                                double inVelocity, double outVelocity) {
    if (index < 0 || index >= static_cast<int>(keyframes.size())) return;

    Keyframe& kf = keyframes[index];
    kf.setBezierHandles(inVelocity, outVelocity);

    auto& interp = kf.getInterpolationData();
    interp.temporalIn.inX = inInfluence * 0.01;
    interp.temporalIn.inY = 1.0 - inVelocity * 0.01;
    interp.temporalOut.outX = outInfluence * 0.01;
    interp.temporalOut.outY = outVelocity * 0.01;
}

// ======================== Smooth Keyframes ========================

std::vector<Keyframe> KeyframeOperations::smoothKeyframes(const std::vector<Keyframe>& keyframes,
                                                          double spatialPrecision, double temporalPrecision) {
    if (keyframes.size() <= 2) return keyframes;

    std::vector<Keyframe> result = keyframes;
    int iterations = static_cast<int>(temporalPrecision);
    if (iterations < 1) iterations = 1;
    if (iterations > 10) iterations = 10;

    // Apply Laplacian smoothing
    for (int iter = 0; iter < iterations; iter++) {
        std::vector<Keyframe> smoothed = result;
        for (size_t i = 1; i + 1 < result.size(); i++) {
            double prevVal = result[i - 1].getValue();
            double currVal = result[i].getValue();
            double nextVal = result[i + 1].getValue();

            double smoothFactor = spatialPrecision * 0.01;
            smoothFactor = std::clamp(smoothFactor, 0.0, 0.5);

            double newVal = currVal * (1.0 - smoothFactor * 2.0) +
                           (prevVal + nextVal) * smoothFactor;
            smoothed[i].setValue(newVal);
        }
        result = smoothed;
    }

    return result;
}

// ======================== Simplify Keyframes ========================

static double perpendicularDist(const Keyframe& kf, const Keyframe& kf1, const Keyframe& kf2) {
    double dx = kf2.getTime() - kf1.getTime();
    double dy = kf2.getValue() - kf1.getValue();
    double lenSq = dx * dx + dy * dy;
    if (lenSq < 1e-12) {
        double dtx = kf.getTime() - kf1.getTime();
        double dty = kf.getValue() - kf1.getValue();
        return std::sqrt(dtx * dtx + dty * dty);
    }
    double t = std::clamp(((kf.getTime() - kf1.getTime()) * dx + (kf.getValue() - kf1.getValue()) * dy) / lenSq,
                          0.0, 1.0);
    double projX = kf1.getTime() + t * dx;
    double projY = kf1.getValue() + t * dy;
    double dtx = kf.getTime() - projX;
    double dty = kf.getValue() - projY;
    return std::sqrt(dtx * dtx + dty * dty);
}

static void rdpSimplifyKF(const std::vector<Keyframe>& kfs, double tolerance,
                           std::vector<Keyframe>& result, size_t start, size_t end) {
    if (end <= start + 1) {
        if (start < kfs.size()) result.push_back(kfs[start]);
        return;
    }
    double maxDist = 0.0;
    size_t maxIdx = start;
    for (size_t i = start + 1; i < end; i++) {
        double d = perpendicularDist(kfs[i], kfs[start], kfs[end]);
        if (d > maxDist) { maxDist = d; maxIdx = i; }
    }
    if (maxDist > tolerance) {
        rdpSimplifyKF(kfs, tolerance, result, start, maxIdx);
        rdpSimplifyKF(kfs, tolerance, result, maxIdx, end);
    } else {
        result.push_back(kfs[start]);
    }
}

std::vector<Keyframe> KeyframeOperations::simplifyKeyframes(const std::vector<Keyframe>& keyframes,
                                                            double tolerance, int maxKeyframes) {
    if (keyframes.size() <= 2) return keyframes;

    std::vector<Keyframe> result;
    rdpSimplifyKF(keyframes, tolerance, result, 0, keyframes.size() - 1);
    result.push_back(keyframes.back());

    // If still too many, thin by maxKeyframes
    if (maxKeyframes > 0 && static_cast<int>(result.size()) > maxKeyframes) {
        std::vector<Keyframe> thinned;
        double step = static_cast<double>(result.size() - 1) / static_cast<double>(maxKeyframes - 1);
        for (int i = 0; i < maxKeyframes; i++) {
            int idx = std::min(static_cast<int>(std::round(i * step)),
                               static_cast<int>(result.size()) - 1);
            thinned.push_back(result[idx]);
        }
        return thinned;
    }

    return result;
}

// ======================== Auto-Ease ========================

static void computeAutoBezierHandles(const std::vector<Keyframe>& kfs, int index,
                                     double& inInfluence, double& outInfluence) {
    if (index < 0 || index >= static_cast<int>(kfs.size())) return;

    double prevTime = (index > 0) ? kfs[index - 1].getTime() : kfs[index].getTime() - 1.0;
    double currTime = kfs[index].getTime();
    double nextTime = (index + 1 < static_cast<int>(kfs.size())) ? kfs[index + 1].getTime()
                                                                 : kfs[index].getTime() + 1.0;

    double totalTime = nextTime - prevTime;
    if (totalTime < 1e-12) totalTime = 1.0;

    double prevDelta = currTime - prevTime;
    double nextDelta = nextTime - currTime;

    // Auto bezier: 1/3 influence based on time ratio
    inInfluence = (prevDelta / totalTime) * 100.0;
    outInfluence = (nextDelta / totalTime) * 100.0;
    inInfluence = std::clamp(inInfluence, 10.0, 90.0);
    outInfluence = std::clamp(outInfluence, 10.0, 90.0);
}

void KeyframeOperations::autoEase(std::vector<Keyframe>& keyframes, const std::vector<int>& indices) {
    for (int idx : indices) {
        if (idx < 0 || idx >= static_cast<int>(keyframes.size())) continue;
        keyframes[idx].setInterpolation(InterpolationType::EaseInOut);
        double inInf, outInf;
        computeAutoBezierHandles(keyframes, idx, inInf, outInf);
        auto& interp = keyframes[idx].getInterpolationData();
        interp.temporalType = InterpolationType::EaseInOut;
        interp.temporalIn.inX = inInf * 0.01;
        interp.temporalIn.inY = 1.0 - 0.33;
        interp.temporalOut.outX = outInf * 0.01;
        interp.temporalOut.outY = 0.33;
    }
}

void KeyframeOperations::easeIn(std::vector<Keyframe>& keyframes, const std::vector<int>& indices) {
    for (int idx : indices) {
        if (idx < 0 || idx >= static_cast<int>(keyframes.size())) continue;
        keyframes[idx].setInterpolation(InterpolationType::EaseIn);
        auto& interp = keyframes[idx].getInterpolationData();
        interp.temporalType = InterpolationType::EaseIn;
    }
}

void KeyframeOperations::easeOut(std::vector<Keyframe>& keyframes, const std::vector<int>& indices) {
    for (int idx : indices) {
        if (idx < 0 || idx >= static_cast<int>(keyframes.size())) continue;
        keyframes[idx].setInterpolation(InterpolationType::EaseOut);
        auto& interp = keyframes[idx].getInterpolationData();
        interp.temporalType = InterpolationType::EaseOut;
    }
}

// ======================== Set Interpolation ========================

void KeyframeOperations::setKeyframeInterpolation(std::vector<Keyframe>& keyframes,
                                                   const std::vector<int>& indices,
                                                   int spatialType, int temporalType) {
    auto toInterpType = [](int type) -> InterpolationType {
        switch (type) {
            case 0: return InterpolationType::Linear;
            case 1: return InterpolationType::Bezier;
            case 2: return InterpolationType::EaseIn;
            case 3: return InterpolationType::EaseOut;
            case 4: return InterpolationType::EaseInOut;
            case 5: return InterpolationType::Hold;
            case 6: return InterpolationType::ContinuousBezier;
            case 7: return InterpolationType::AutoBezier;
            default: return InterpolationType::Linear;
        }
    };

    InterpolationType tempType = toInterpType(temporalType);
    InterpolationType spatType = toInterpType(spatialType);

    for (int idx : indices) {
        if (idx < 0 || idx >= static_cast<int>(keyframes.size())) continue;
        keyframes[idx].setInterpolation(tempType);
        keyframes[idx].getInterpolationData().temporalType = tempType;
        keyframes[idx].getInterpolationData().spatialType = spatType;
    }
}

// ======================== Auto Bezier ========================

void KeyframeOperations::autoBezier(std::vector<Keyframe>& keyframes) {
    for (size_t i = 0; i < keyframes.size(); i++) {
        keyframes[i].setInterpolation(InterpolationType::AutoBezier);
        auto& interp = keyframes[i].getInterpolationData();
        interp.temporalType = InterpolationType::AutoBezier;

        double inInf, outInf;
        computeAutoBezierHandles(keyframes, static_cast<int>(i), inInf, outInf);

        interp.temporalIn.inX = inInf * 0.01;
        interp.temporalIn.inY = 0.0;
        interp.temporalOut.outX = outInf * 0.01;
        interp.temporalOut.outY = 1.0;
        interp.continuous = true;
    }
}

// ======================== Roving Keyframes ========================

void KeyframeOperations::roveKeyframes(std::vector<Keyframe>& keyframes, const std::vector<int>& indices) {
    if (indices.empty() || keyframes.size() < 2) return;

    // Roving keyframes distribute evenly in time between fixed keyframes
    // Find the fixed keyframes (non-roving)
    std::vector<int> sortedIdx = indices;
    std::sort(sortedIdx.begin(), sortedIdx.end());

    // Get the time range of roving section
    int firstRoving = sortedIdx.front();
    int lastRoving = sortedIdx.back();

    double startTime = (firstRoving > 0) ? keyframes[firstRoving - 1].getTime() : keyframes.front().getTime();
    double endTime = (lastRoving + 1 < static_cast<int>(keyframes.size()))
                         ? keyframes[lastRoving + 1].getTime()
                         : keyframes.back().getTime();

    int rovingCount = static_cast<int>(sortedIdx.size()) + 2; // include endpoints
    double step = (endTime - startTime) / static_cast<double>(rovingCount - 1);

    for (size_t i = 0; i < sortedIdx.size(); i++) {
        double newTime = startTime + step * static_cast<double>(i + 1);
        keyframes[sortedIdx[i]].setTime(newTime);
    }
}

// ======================== Find Nearest Keyframe ========================

int KeyframeOperations::findNearestKeyframe(const std::vector<Keyframe>& keyframes, double time) {
    if (keyframes.empty()) return -1;
    int nearest = 0;
    double minDist = std::abs(keyframes[0].getTime() - time);
    for (size_t i = 1; i < keyframes.size(); i++) {
        double dist = std::abs(keyframes[i].getTime() - time);
        if (dist < minDist) {
            minDist = dist;
            nearest = static_cast<int>(i);
        }
    }
    return nearest;
}

// ======================== Get Keyframe Velocity ========================

double KeyframeOperations::getKeyframeVelocity(const std::vector<Keyframe>& keyframes, double time) {
    if (keyframes.size() < 2) return 0.0;

    // Find surrounding keyframes
    int idx = findNearestKeyframe(keyframes, time);
    if (idx < 0) return 0.0;

    // If between two keyframes, compute instantaneous velocity
    int prevIdx = (idx > 0) ? idx - 1 : 0;
    int nextIdx = (idx + 1 < static_cast<int>(keyframes.size())) ? idx + 1 : idx;

    if (prevIdx == nextIdx) return 0.0;

    double dt = keyframes[nextIdx].getTime() - keyframes[prevIdx].getTime();
    if (dt < 1e-12) return 0.0;

    double dv = keyframes[nextIdx].getValue() - keyframes[prevIdx].getValue();
    return dv / dt;
}

} // namespace FreeEffect
