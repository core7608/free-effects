#pragma once
#include "../effect.h"

namespace FreeEffect {

class WindEffect : public Effect {
public:
    WindEffect();
    std::string getName() const override { return "Wind"; }
    std::string getCategory() const override { return "Stylize"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
