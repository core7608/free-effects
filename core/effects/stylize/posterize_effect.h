#pragma once
#include "../effect.h"

namespace FreeEffect {

class PosterizeEffect : public Effect {
public:
    PosterizeEffect();
    std::string getName() const override { return "Posterize"; }
    std::string getCategory() const override { return "Stylize"; }
    std::vector<ParameterGroup> getParameterGroups() const override;
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
