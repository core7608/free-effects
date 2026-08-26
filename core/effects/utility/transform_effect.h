#pragma once
#include "../effect.h"

namespace FreeEffect {

class TransformEffect : public Effect {
public:
    TransformEffect();
    std::string getName() const override { return "Transform"; }
    std::string getCategory() const override { return "Utility"; }
    std::vector<ParameterGroup> getParameterGroups() const override;
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
