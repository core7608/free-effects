#pragma once
#include "../effect.h"

namespace FreeEffect {

class AdvancedLightningEffect : public Effect {
public:
    AdvancedLightningEffect();
    std::string getName() const override { return "Advanced Lightning"; }
    std::string getCategory() const override { return "Generate"; }
    std::vector<ParameterGroup> getParameterGroups() const override;
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
