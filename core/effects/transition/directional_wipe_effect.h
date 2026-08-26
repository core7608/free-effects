#pragma once
#include "../effect.h"

namespace FreeEffect {

class DirectionalWipeEffect : public Effect {
public:
    DirectionalWipeEffect();
    std::string getName() const override { return "Directional Wipe"; }
    std::string getCategory() const override { return "Transition"; }
    std::vector<ParameterGroup> getParameterGroups() const override;
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
