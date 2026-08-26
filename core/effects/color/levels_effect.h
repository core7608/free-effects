#pragma once
#include "../effect.h"

namespace FreeEffect {

class LevelsEffect : public Effect {
public:
    LevelsEffect();
    std::string getName() const override { return "Levels"; }
    std::string getCategory() const override { return "Color Correction"; }
    std::vector<ParameterGroup> getParameterGroups() const override;
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
