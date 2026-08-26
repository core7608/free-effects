#pragma once
#include "../effect.h"

namespace FreeEffect {

class TimeDisplacementEffect : public Effect {
public:
    TimeDisplacementEffect();
    std::string getName() const override { return "Time Displacement"; }
    std::string getCategory() const override { return "Utility"; }
    std::vector<ParameterGroup> getParameterGroups() const override;
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
