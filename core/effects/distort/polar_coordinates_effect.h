#pragma once
#include "../effect.h"

namespace FreeEffect {

class PolarCoordinatesEffect : public Effect {
public:
    PolarCoordinatesEffect();
    std::string getName() const override { return "Polar Coordinates"; }
    std::string getCategory() const override { return "Distort"; }
    std::vector<ParameterGroup> getParameterGroups() const override;
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
