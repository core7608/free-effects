#pragma once
#include "../effect.h"

namespace FreeEffect {

class CCBenderEffect : public Effect {
public:
    CCBenderEffect();
    std::string getName() const override { return "CC Bender"; }
    std::string getCategory() const override { return "Distort"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
