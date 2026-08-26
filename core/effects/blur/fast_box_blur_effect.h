#pragma once
#include "../effect.h"

namespace FreeEffect {

class FastBoxBlurEffect : public Effect {
public:
    FastBoxBlurEffect();
    std::string getName() const override { return "Fast Box Blur"; }
    std::string getCategory() const override { return "Blur & Sharpen"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
