#pragma once
#include "../effect.h"

namespace FreeEffect {

class GlitchTransition : public Effect {
public:
    GlitchTransition();
    std::string getName() const override { return "Glitch Transition"; }
    std::string getCategory() const override { return "Transition"; }
    std::vector<ParameterGroup> getParameterGroups() const override;
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
