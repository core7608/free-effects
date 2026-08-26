#pragma once
#include "../effect.h"

namespace FreeEffect {

class CompoundBlurEffect : public Effect {
public:
    CompoundBlurEffect();
    std::string getName() const override { return "Compound Blur"; }
    std::string getCategory() const override { return "Blur & Sharpen"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
