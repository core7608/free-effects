#pragma once
#include "../effect.h"

namespace FreeEffect {

class SpherizeEffect : public Effect {
public:
    SpherizeEffect();
    std::string getName() const override { return "Spherize"; }
    std::string getCategory() const override { return "Distort"; }
    std::vector<ParameterGroup> getParameterGroups() const override;
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
