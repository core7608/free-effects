#pragma once
#include "../effect.h"

namespace FreeEffect {

class OpticalCompensationEffect : public Effect {
public:
    OpticalCompensationEffect();
    std::string getName() const override { return "Optical Compensation"; }
    std::string getCategory() const override { return "Distort"; }
    std::vector<ParameterGroup> getParameterGroups() const override;
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
