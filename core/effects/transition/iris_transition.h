#pragma once
#include "../effect.h"

namespace FreeEffect {

class IrisTransition : public Effect {
public:
    IrisTransition();
    std::string getName() const override { return "Iris Transition"; }
    std::string getCategory() const override { return "Transition"; }
    std::vector<ParameterGroup> getParameterGroups() const override;
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
