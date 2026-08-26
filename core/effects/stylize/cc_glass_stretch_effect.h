#pragma once
#include "../effect.h"

namespace FreeEffect {

class CCGlassStretchEffect : public Effect {
public:
    CCGlassStretchEffect();
    std::string getName() const override { return "CC Glass Stretch"; }
    std::string getCategory() const override { return "Stylize"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
