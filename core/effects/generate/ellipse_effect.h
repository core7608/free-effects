#pragma once
#include "../effect.h"

namespace FreeEffect {

class EllipseEffect : public Effect {
public:
    EllipseEffect();
    std::string getName() const override { return "Ellipse"; }
    std::string getCategory() const override { return "Generate"; }
    std::vector<ParameterGroup> getParameterGroups() const override;
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
