#pragma once
#include "../effect.h"

namespace FreeEffect {

class CCMrSmoothieEffect : public Effect {
public:
    CCMrSmoothieEffect();
    std::string getName() const override { return "CC Mr Smoothie"; }
    std::string getCategory() const override { return "Stylize"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
