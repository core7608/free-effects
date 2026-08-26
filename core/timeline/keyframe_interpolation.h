#pragma once

#include "types.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

struct BezierHandle {
    double inX = 0.0;
    double inY = 0.0;
    double outX = 1.0;
    double outY = 1.0;
};

struct KeyframeInterpolation {
    InterpolationType temporalType = InterpolationType::Linear;
    InterpolationType spatialType = InterpolationType::Linear;
    BezierHandle temporalIn;
    BezierHandle temporalOut;
    BezierHandle spatialIn;
    BezierHandle spatialOut;
    bool continuous = false;

    double getTemporalValue(double progress) const {
        progress = std::clamp(progress, 0.0, 1.0);

        switch (temporalType) {
            case InterpolationType::Linear:
                return progress;

            case InterpolationType::EaseIn: {
                double t = progress;
                return t * t * t;
            }

            case InterpolationType::EaseOut: {
                double t = 1.0 - progress;
                return 1.0 - t * t * t;
            }

            case InterpolationType::EaseInOut: {
                double t = progress;
                if (t < 0.5) {
                    return 4.0 * t * t * t;
                } else {
                    double f = -2.0 * t + 2.0;
                    return 1.0 - (f * f * f) / 2.0;
                }
            }

            case InterpolationType::Bezier:
            case InterpolationType::ContinuousBezier:
            case InterpolationType::AutoBezier: {
                double p0x = 0.0, p0y = 0.0;
                double p1x = temporalOut.outX, p1y = temporalOut.outY;
                double p2x = temporalIn.inX, p2y = temporalIn.inY;
                double p3x = 1.0, p3y = 1.0;

                double t = progress;
                for (int i = 0; i < 8; ++i) {
                    double u = 1.0 - t;
                    double bx = u * u * u * p0x + 3.0 * u * u * t * p1x +
                                3.0 * u * t * t * p2x + t * t * t * p3x;
                    double dx = 3.0 * u * u * (p1x - p0x) + 6.0 * u * t * (p2x - p1x) +
                                3.0 * t * t * (p3x - p2x);
                    if (std::abs(dx) < 1e-8) break;
                    t -= (bx - progress) / dx;
                    t = std::clamp(t, 0.0, 1.0);
                }

                double u = 1.0 - t;
                return u * u * u * p0y + 3.0 * u * u * t * p1y +
                       3.0 * u * t * t * p2y + t * t * t * p3y;
            }

            case InterpolationType::Hold:
                return 0.0;

            default:
                return progress;
        }
    }

    static BezierHandle computeAutoBezier(BezierHandle prev, BezierHandle next) {
        double prevOutY = prev.outY;
        double nextInY = next.inY;

        double dy = nextInY - prevOutY;
        double dx = 1.0;

        BezierHandle result;
        result.inX = 1.0 / 3.0;
        result.inY = std::clamp(prevOutY + dy * 0.25, 0.0, 1.0);
        result.outX = 2.0 / 3.0;
        result.outY = std::clamp(nextInY - dy * 0.25, 0.0, 1.0);

        return result;
    }
};

} // namespace FreeEffect
