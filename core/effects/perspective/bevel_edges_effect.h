#pragma once
#include "../effect.h"

namespace FreeEffect {

class BevelEdgesEffect : public Effect {
public:
    BevelEdgesEffect();
    std::string getName() const override { return "Bevel Edges"; }
    std::string getCategory() const override { return "Perspective"; }
    std::vector<ParameterGroup> getParameterGroups() const override;
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
