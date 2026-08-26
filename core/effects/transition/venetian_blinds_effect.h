#pragma once
#include "../effect.h"

namespace FreeEffect {

class VenetianBlindsEffect : public Effect {
public:
    VenetianBlindsEffect();
    std::string getName() const override { return "Venetian Blinds"; }
    std::string getCategory() const override { return "Transition"; }
    std::vector<ParameterGroup> getParameterGroups() const override;
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
