#pragma once
#include "../effect.h"

namespace FreeEffect {

class StrokeEffect : public Effect {
public:
    StrokeEffect();
    std::string getName() const override { return "Stroke"; }
    std::string getCategory() const override { return "Generate"; }
    std::vector<ParameterGroup> getParameterGroups() const override;
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
