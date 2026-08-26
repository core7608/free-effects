#pragma once

namespace FreeEffect {

enum class TrackMatteMode {
    None,
    AlphaMatte,
    AlphaInvertedMatte,
    LumaMatte,
    LumaInvertedMatte
};

struct TrackMatteReference {
    int matteLayerIndex = -1;
    TrackMatteMode mode = TrackMatteMode::None;

    bool isEnabled() const { return mode != TrackMatteMode::None && matteLayerIndex >= 0; }
};

} // namespace FreeEffect
