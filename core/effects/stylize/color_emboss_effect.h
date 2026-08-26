#pragma once
#include "../effect.h"

namespace FreeEffect {

class ColorEmbossEffect : public Effect {
public:
    ColorEmbossEffect();
    std::string getName() const override { return "Color Emboss"; }
    std::string getCategory() const override { return "Stylize"; }
    std::vector<ParameterGroup> getParameterGroups() const override;
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
