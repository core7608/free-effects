#pragma once
#include "../effect.h"

namespace FreeEffect {

class WipeLeftEffect : public Effect {
public:
    WipeLeftEffect();
    std::string getName() const override { return "Wipe Left"; }
    std::string getCategory() const override { return "Transition"; }
    std::vector<ParameterGroup> getParameterGroups() const override;
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
