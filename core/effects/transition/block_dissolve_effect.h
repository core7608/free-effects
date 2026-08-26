#pragma once
#include "../effect.h"

namespace FreeEffect {

class BlockDissolveEffect : public Effect {
public:
    BlockDissolveEffect();
    std::string getName() const override { return "Block Dissolve"; }
    std::string getCategory() const override { return "Transition"; }
    std::vector<ParameterGroup> getParameterGroups() const override;
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
