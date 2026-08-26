#pragma once
#include "../effect.h"

namespace FreeEffect {

class CCPowerPinEffect : public Effect {
public:
    CCPowerPinEffect();
    std::string getName() const override { return "CC Power Pin"; }
    std::string getCategory() const override { return "Distort"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
