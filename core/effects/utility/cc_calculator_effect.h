#pragma once
#include "../effect.h"

namespace FreeEffect {

class CCCalculatorEffect : public Effect {
public:
    CCCalculatorEffect();
    std::string getName() const override { return "CC Calculator"; }
    std::string getCategory() const override { return "Utility"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
