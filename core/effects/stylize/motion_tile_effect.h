#pragma once
#include "../effect.h"

namespace FreeEffect {

class MotionTileEffect : public Effect {
public:
    MotionTileEffect();
    std::string getName() const override { return "Motion Tile"; }
    std::string getCategory() const override { return "Stylize"; }
    std::vector<ParameterGroup> getParameterGroups() const override;
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
