#pragma once
#include "../effect.h"

namespace FreeEffect {

class DepthOfFieldEffect : public Effect {
public:
    DepthOfFieldEffect();
    std::string getName() const override { return "Depth of Field"; }
    std::string getCategory() const override { return "Blur & Sharpen"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
