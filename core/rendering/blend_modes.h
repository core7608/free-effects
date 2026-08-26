#pragma once

#include "../timeline/types.h"
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace FreeEffect {

enum class BlendModeType {
    Normal, Dissolve,
    Darken, Multiply, ColorBurn, LinearBurn, DarkerColor,
    Lighten, Screen, ColorDodge, LinearDodge, LighterColor,
    Overlay, SoftLight, HardLight, VividLight, LinearLight, PinLight, HardMix,
    Difference, Exclusion, Subtract, Divide,
    Hue, Saturation, Color, Luminosity
};

struct HSL {
    double h = 0.0;
    double s = 0.0;
    double l = 0.0;
};

HSL rgbToHsl(uint8_t r, uint8_t g, uint8_t b);
void hslToRgb(const HSL& hsl, uint8_t& r, uint8_t& g, uint8_t& b);

void applyBlendMode(BlendModeType mode, const uint8_t src[4], uint8_t dst[4], double opacity);

BlendModeType mapBlendMode(BlendMode mode);

} // namespace FreeEffect
