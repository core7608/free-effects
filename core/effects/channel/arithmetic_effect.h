#pragma once
#include "../effect.h"

namespace FreeEffect {

class ArithmeticEffect : public Effect {
public:
    ArithmeticEffect();
    std::string getName() const override { return "Arithmetic"; }
    std::string getCategory() const override { return "Channel"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
