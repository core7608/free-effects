#pragma once
#include "../effect.h"

namespace FreeEffect {

class CCSparkleEffect : public Effect {
public:
    CCSparkleEffect();
    std::string getName() const override { return "CC Sparkle"; }
    std::string getCategory() const override { return "Stylize"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
