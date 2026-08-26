#pragma once
#include "../effect.h"

namespace FreeEffect {

class CCSnowEffect : public Effect {
public:
    CCSnowEffect();
    std::string getName() const override { return "CC Snow"; }
    std::string getCategory() const override { return "Generate"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
