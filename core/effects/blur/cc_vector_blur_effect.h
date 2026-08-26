#pragma once
#include "../effect.h"

namespace FreeEffect {

class CCVectorBlurEffect : public Effect {
public:
    CCVectorBlurEffect();
    std::string getName() const override { return "CC Vector Blur"; }
    std::string getCategory() const override { return "Blur & Sharpen"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
