#pragma once
#include "../effect.h"

namespace FreeEffect {

class WriteOnEffect : public Effect {
public:
    WriteOnEffect();
    std::string getName() const override { return "Write-On"; }
    std::string getCategory() const override { return "Generate"; }
    std::vector<ParameterGroup> getParameterGroups() const override;
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
