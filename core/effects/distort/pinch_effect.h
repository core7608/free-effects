#pragma once
#include "../effect.h"

namespace FreeEffect {

class PinchEffect : public Effect {
public:
    PinchEffect();
    std::string getName() const override { return "Pinch"; }
    std::string getCategory() const override { return "Distort"; }
    std::vector<ParameterGroup> getParameterGroups() const override;
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
