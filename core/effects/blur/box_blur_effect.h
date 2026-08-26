#pragma once
#include "../effect.h"

namespace FreeEffect {

class BoxBlurEffect : public Effect {
public:
    BoxBlurEffect();
    std::string getName() const override { return "Box Blur"; }
    std::string getCategory() const override { return "Blur & Sharpen"; }
    std::vector<ParameterGroup> getParameterGroups() const override;
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
