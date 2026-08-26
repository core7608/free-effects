#pragma once
#include "../effect.h"

namespace FreeEffect {

class FindEdgesEffect : public Effect {
public:
    FindEdgesEffect();
    std::string getName() const override { return "Find Edges"; }
    std::string getCategory() const override { return "Stylize"; }
    std::vector<ParameterGroup> getParameterGroups() const override;
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
