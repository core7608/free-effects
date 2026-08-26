#pragma once
#include "../effect.h"

namespace FreeEffect {

class RadialWipeEffect : public Effect {
public:
    RadialWipeEffect();
    std::string getName() const override { return "Radial Wipe"; }
    std::string getCategory() const override { return "Transition"; }
    std::vector<ParameterGroup> getParameterGroups() const override;
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
