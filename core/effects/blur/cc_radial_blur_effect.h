#pragma once
#include "../effect.h"

namespace FreeEffect {

class CCRadialBlurEffect : public Effect {
public:
    CCRadialBlurEffect();
    std::string getName() const override { return "CC Radial Blur"; }
    std::string getCategory() const override { return "Blur & Sharpen"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
