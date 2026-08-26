#pragma once
#include "../effect.h"

namespace FreeEffect {

class MotionBlurEffect : public Effect {
public:
    MotionBlurEffect();
    std::string getName() const override { return "Motion Blur"; }
    std::string getCategory() const override { return "Blur & Sharpen"; }
    std::vector<ParameterGroup> getParameterGroups() const override;
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
