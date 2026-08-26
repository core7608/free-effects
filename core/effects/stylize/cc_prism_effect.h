#pragma once
#include "../effect.h"

namespace FreeEffect {

class CCPrismEffect : public Effect {
public:
    CCPrismEffect();
    std::string getName() const override { return "CC Prism"; }
    std::string getCategory() const override { return "Stylize"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
