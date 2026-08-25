#pragma once

#include <chrono>
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
    uint32_t clockSeqLow = dis(gen) & 0xFFFF;
    // Node: 48 bits = 12 hex chars
    uint64_t node = (static_cast<uint64_t>(dis(gen) & 0xFFFFFF) << 24) | dis(gen);
    
    char buf[37];
    snprintf(buf, sizeof(buf), "%08x-%04x-%04x-%04x-%04x%08x",
             timeLow, timeMid, timeHi, clockSeq, clockSeqLow,
             static_cast<uint32_t>(node >> 16),
             static_cast<uint32_t>(node & 0xFFFF));
    
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
    Light
};

enum class BlendMode {
    Normal,
    Add,
    Multiply,
    Screen
};

enum class InterpolationType {
    Linear,
    EaseIn,
    EaseOut,
    EaseInOut,
    Bezier,
    Hold
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
};

} // namespace FreeEffect
