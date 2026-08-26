#pragma once
#include "../effect.h"

namespace FreeEffect {

class WhiteBalanceEffect : public Effect {
public:
    WhiteBalanceEffect();
    std::string getName() const override { return "White Balance"; }
    std::string getCategory() const override { return "Color Correction"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
