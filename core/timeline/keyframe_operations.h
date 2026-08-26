#pragma once

#include "keyframe.h"
#include "property_track.h"
#include "layer.h"
#include <vector>
#include <memory>

namespace FreeEffect {

class KeyframeOperations {
public:
    // Convert audio amplitude to keyframes
    static std::vector<Keyframe> audioToKeyframes(const std::vector<float>& audioSamples,
                                                   int sampleRate, double threshold);

    // Convert expression to keyframes
    static std::vector<Keyframe> expressionToKeyframes(const PropertyTrack& track,
                                                        double startTime, double endTime,
                                                        double sampleRate);

    // Sequence layers in time
    static void sequenceLayers(std::vector<std::shared_ptr<Layer>>& layers,
                               double gap, int order, bool overlap);

    // Apply keyframe velocity
    static void applyKeyframeVelocity(std::vector<Keyframe>& keyframes,
                                       int index, double inInfluence, double outInfluence,
                                       double inVelocity, double outVelocity);

    // Smooth keyframes (Smoother)
    static std::vector<Keyframe> smoothKeyframes(const std::vector<Keyframe>& keyframes,
                                                  double spatialPrecision, double temporalPrecision);

    // Simplify keyframes
    static std::vector<Keyframe> simplifyKeyframes(const std::vector<Keyframe>& keyframes,
                                                    double tolerance, int maxKeyframes);

    // Auto-ease (Flow)
    static void autoEase(std::vector<Keyframe>& keyframes, const std::vector<int>& indices);
    static void easeIn(std::vector<Keyframe>& keyframes, const std::vector<int>& indices);
    static void easeOut(std::vector<Keyframe>& keyframes, const std::vector<int>& indices);

    // Keyframe interpolation conversion
    static void setKeyframeInterpolation(std::vector<Keyframe>& keyframes,
                                          const std::vector<int>& indices,
                                          int spatialType, int temporalType);

    // Bezier handle auto-computation
    static void autoBezier(std::vector<Keyframe>& keyframes);

    // Roving keyframes
    static void roveKeyframes(std::vector<Keyframe>& keyframes, const std::vector<int>& indices);

    // Find nearest keyframe
    static int findNearestKeyframe(const std::vector<Keyframe>& keyframes, double time);

    // Get keyframe velocity at time
    static double getKeyframeVelocity(const std::vector<Keyframe>& keyframes, double time);
};

} // namespace FreeEffect
