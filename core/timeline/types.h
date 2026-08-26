#pragma once

#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <random>
#include <string>

namespace FreeEffect {

// Unique ID for assets, layers, compositions
using UUID = std::string;

inline UUID generateUUID() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<uint32_t> dis(0, 0xFFFFFFFF);
    
    uint32_t timeLow = dis(gen);
    uint32_t timeMid = dis(gen) & 0xFFFF;
    uint32_t timeHi = (dis(gen) & 0x0FFF) | 0x4000;
    uint32_t clockSeq = (dis(gen) & 0x3FFF) | 0x8000;
    // Node: 48 bits = 12 hex chars
    uint64_t node = (static_cast<uint64_t>(dis(gen) & 0xFFFFFF) << 24) | dis(gen);
    
    char buf[37];
    snprintf(buf, sizeof(buf), "%08x-%04x-%04x-%04x-%012llx",
             timeLow, timeMid, timeHi, clockSeq,
             (unsigned long long)(node & 0xFFFFFFFFFFFFULL));
    
    return std::string(buf);
}

struct Resolution {
    int width = 1920;
    int height = 1080;
};

struct FrameRate {
    double fps = 30.0;
    double getFrameDuration() const { return 1.0 / fps; }
};

using TimePoint = std::chrono::duration<double>;

enum class LayerType {
    Video,
    Image,
    Audio,
    Text,
    Shape,
    Null,
    Solid,
    Adjustment,
    Camera,
    Light,
    Precomp
};

enum class BlendMode {
    Normal,
    Add,
    Multiply,
    Screen,
    Darken,
    Lighten,
    Overlay,
    SoftLight,
    HardLight,
    ColorBurn,
    ColorDodge,
    LinearBurn,
    LinearDodge,
    VividLight,
    LinearLight,
    PinLight,
    HardMix,
    Difference,
    Exclusion,
    Subtract,
    Divide,
    Hue,
    Saturation,
    Color,
    Luminosity,
    Dissolve,
    DarkerColor,
    LighterColor
};

enum class InterpolationType {
    Linear,
    EaseIn,
    EaseOut,
    EaseInOut,
    Bezier,
    Hold,
    ContinuousBezier,
    AutoBezier
};

enum class AssetStatus {
    Available,
    Missing,
    Corrupted
};

struct Color {
    double r = 0.0;
    double g = 0.0;
    double b = 0.0;
    double a = 1.0;
};

struct Vec2 {
    double x = 0.0;
    double y = 0.0;
};

struct Vec3 {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;

    Vec3() = default;
    Vec3(double x, double y, double z) : x(x), y(y), z(z) {}

    Vec3 operator+(const Vec3& rhs) const { return {x + rhs.x, y + rhs.y, z + rhs.z}; }
    Vec3 operator-(const Vec3& rhs) const { return {x - rhs.x, y - rhs.y, z - rhs.z}; }
    Vec3 operator*(double s) const { return {x * s, y * s, z * s}; }
    Vec3 operator*(const Vec3& rhs) const { return {x * rhs.x, y * rhs.y, z * rhs.z}; }

    Vec3& operator+=(const Vec3& rhs) { x += rhs.x; y += rhs.y; z += rhs.z; return *this; }
    Vec3& operator-=(const Vec3& rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; return *this; }
    Vec3& operator*=(double s) { x *= s; y *= s; z *= s; return *this; }

    Vec3 operator-() const { return {-x, -y, -z}; }

    double dot(const Vec3& rhs) const { return x * rhs.x + y * rhs.y + z * rhs.z; }

    Vec3 cross(const Vec3& rhs) const {
        return {y * rhs.z - z * rhs.y,
                z * rhs.x - x * rhs.z,
                x * rhs.y - y * rhs.x};
    }

    double length() const { return std::sqrt(x * x + y * y + z * z); }

    Vec3 normalized() const {
        double len = length();
        if (len < 1e-12) return {0.0, 0.0, 0.0};
        return *this * (1.0 / len);
    }
};

} // namespace FreeEffect
