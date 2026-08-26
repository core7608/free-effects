#pragma once
#include "../effect.h"

namespace FreeEffect {

class DifferenceMatteEffect : public Effect {
public:
    DifferenceMatteEffect();
    std::string getName() const override { return "Difference Matte"; }
    std::string getCategory() const override { return "Keying"; }
    std::vector<ParameterGroup> getParameterGroups() const override;
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
