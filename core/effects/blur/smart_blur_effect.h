#pragma once
#include "../effect.h"

namespace FreeEffect {

class SmartBlurEffect : public Effect {
public:
    SmartBlurEffect();
    std::string getName() const override { return "Smart Blur"; }
    std::string getCategory() const override { return "Blur & Sharpen"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
