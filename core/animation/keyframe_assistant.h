#pragma once

#include "../timeline/keyframe.h"
#include "../timeline/property_track.h"
#include <vector>

namespace FreeEffect {

struct KeyframeVelocity {
    double time = 0;
    double inVelocity = 0;
    double outVelocity = 0;
    double inInfluence = 33.3;
    double outInfluence = 33.3;
    int inType = 0;
    int outType = 0;
};

class KeyframeAssistant {
public:
    static void easyEase(std::vector<Keyframe>& keyframes,
                         const std::vector<int>& selectedIndices);

    static void easyEaseIn(std::vector<Keyframe>& keyframes,
                           const std::vector<int>& selectedIndices);

    static void easyEaseOut(std::vector<Keyframe>& keyframes,
                            const std::vector<int>& selectedIndices);

    static std::vector<Keyframe> exponentialScale(
        double startValue, double endValue,
        double startTime, double endTime, int numKeyframes);

    static void timeReverseKeyframes(std::vector<Keyframe>& keyframes,
                                     double duration);

    static void toggleHold(std::vector<Keyframe>& keyframes,
                           const std::vector<int>& selectedIndices);

    static Keyframe addKeyframeAtTime(const PropertyTrack& track, double time);

    static int findNextKeyframe(const std::vector<Keyframe>& keyframes, double currentTime);
    static int findPrevKeyframe(const std::vector<Keyframe>& keyframes, double currentTime);

    static std::vector<KeyframeVelocity> computeVelocities(
        const std::vector<Keyframe>& keyframes);

    static void roveVertices(std::vector<Keyframe>& keyframes,
                             const std::vector<int>& selectedIndices);

    static void convertToLinear(std::vector<Keyframe>& keyframes,
                                const std::vector<int>& selectedIndices);

    static void convertToBezier(std::vector<Keyframe>& keyframes,
                                const std::vector<int>& selectedIndices);

    static void convertToHold(std::vector<Keyframe>& keyframes,
                              const std::vector<int>& selectedIndices);
};

} // namespace FreeEffect
