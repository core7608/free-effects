#pragma once
#include "../effect.h"

namespace FreeEffect {

class CCKaleidaEffect : public Effect {
public:
    CCKaleidaEffect();
    std::string getName() const override { return "CC Kaleida"; }
    std::string getCategory() const override { return "Stylize"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
