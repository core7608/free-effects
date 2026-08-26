#pragma once
#include "../effect.h"

namespace FreeEffect {

class GlowEffect : public Effect {
public:
    GlowEffect();
    std::string getName() const override { return "Glow"; }
    std::string getCategory() const override { return "Stylize"; }
    std::vector<ParameterGroup> getParameterGroups() const override;
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
