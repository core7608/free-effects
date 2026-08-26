#pragma once
#include "../effect.h"

namespace FreeEffect {

class StrobeEffect : public Effect {
public:
    StrobeEffect();
    std::string getName() const override { return "Strobe"; }
    std::string getCategory() const override { return "Stylize"; }
    std::vector<ParameterGroup> getParameterGroups() const override;
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
