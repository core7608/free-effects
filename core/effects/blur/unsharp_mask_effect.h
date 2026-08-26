#pragma once
#include "../effect.h"

namespace FreeEffect {

class UnsharpMaskEffect : public Effect {
public:
    UnsharpMaskEffect();
    std::string getName() const override { return "Unsharp Mask"; }
    std::string getCategory() const override { return "Blur & Sharpen"; }
    std::vector<ParameterGroup> getParameterGroups() const override;
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
