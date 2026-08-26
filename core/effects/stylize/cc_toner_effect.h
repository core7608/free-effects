#pragma once
#include "../effect.h"

namespace FreeEffect {

class CCTonerEffect : public Effect {
public:
    CCTonerEffect();
    std::string getName() const override { return "CC Toner"; }
    std::string getCategory() const override { return "Stylize"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
