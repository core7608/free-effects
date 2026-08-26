#pragma once
#include "../effect.h"

namespace FreeEffect {

class CCCrossBlurEffect : public Effect {
public:
    CCCrossBlurEffect();
    std::string getName() const override { return "CC Cross Blur"; }
    std::string getCategory() const override { return "Blur & Sharpen"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
