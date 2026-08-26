#pragma once
#include "../effect.h"

namespace FreeEffect {

class CCBurnFilmEffect : public Effect {
public:
    CCBurnFilmEffect();
    std::string getName() const override { return "CC Burn Film"; }
    std::string getCategory() const override { return "Stylize"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
