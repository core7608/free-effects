#pragma once
#include "../effect.h"

namespace FreeEffect {

class DisplacementMapEffect : public Effect {
public:
    DisplacementMapEffect();
    std::string getName() const override { return "Displacement Map"; }
    std::string getCategory() const override { return "Distort"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
