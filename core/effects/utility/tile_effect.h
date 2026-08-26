#pragma once
#include "../effect.h"

namespace FreeEffect {

class TileEffect : public Effect {
public:
    TileEffect();
    std::string getName() const override { return "Tile"; }
    std::string getCategory() const override { return "Utility"; }
    std::vector<ParameterGroup> getParameterGroups() const override;
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
