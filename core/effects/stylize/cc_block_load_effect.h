#pragma once
#include "../effect.h"

namespace FreeEffect {

class CCBlockLoadEffect : public Effect {
public:
    CCBlockLoadEffect();
    std::string getName() const override { return "CC Block Load"; }
    std::string getCategory() const override { return "Stylize"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
