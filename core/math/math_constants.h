#pragma once

#ifdef _MSC_VER
    #ifndef _USE_MATH_DEFINES
        #define _USE_MATH_DEFINES
    #endif
#endif

#include <cmath>

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

#ifndef M_PI_2
    #define M_PI_2 1.57079632679489661923
#endif

#ifndef M_PI_4
    #define M_PI_4 0.78539816339744830962
#endif

#ifndef M_SQRT2
    #define M_SQRT2 1.41421356237309504880
#endif

#ifndef M_SQRT1_2
    #define M_SQRT1_2 0.70710678118654752440
#endif

#ifndef M_LN2
    #define M_LN2 0.69314718055994530942
#endif

#ifndef M_LN10
    #define M_LN10 2.30258509299404568402
#endif

#ifndef M_E
    #define M_E 2.71828182845904523536
#endif

namespace FreeEffect {
namespace math {

constexpr double PI      = 3.14159265358979323846;
constexpr double PI_2    = 1.57079632679489661923;
constexpr double PI_4    = 0.78539816339744830962;
constexpr double SQRT2   = 1.41421356237309504880;
constexpr double SQRT1_2 = 0.70710678118654752440;
constexpr double LN2     = 0.69314718055994530942;
constexpr double LN10    = 2.30258509299404568402;
constexpr double E       = 2.71828182845904523536;
constexpr double DEG2RAD = PI / 180.0;
constexpr double RAD2DEG = 180.0 / PI;

inline double degToRad(double degrees) { return degrees * DEG2RAD; }
inline double radToDeg(double radians) { return radians * RAD2DEG; }

} // namespace math
} // namespace FreeEffect
