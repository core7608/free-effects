#pragma once
#include "../effect.h"

namespace FreeEffect {

class ColorKeyEffect : public Effect {
public:
    ColorKeyEffect();
    std::string getName() const override { return "Color Key"; }
    std::string getCategory() const override { return "Keying"; }
    std::vector<ParameterGroup> getParameterGroups() const override;
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
