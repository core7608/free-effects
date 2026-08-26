#pragma once
#include "../effect.h"

namespace FreeEffect {

class BevelAlphaEffect : public Effect {
public:
    BevelAlphaEffect();
    std::string getName() const override { return "Bevel Alpha"; }
    std::string getCategory() const override { return "Stylize"; }
    std::vector<ParameterGroup> getParameterGroups() const override;
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
