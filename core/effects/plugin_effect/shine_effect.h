#pragma once
#include "../effect.h"

namespace FreeEffect {

class ShineEffect : public Effect {
public:
    ShineEffect();
    std::string getName() const override { return "Shine"; }
    std::string getCategory() const override { return "Plugin Effect"; }
    std::string getSubCategory() const override { return "Trapcode"; }
    std::vector<ParameterGroup> getParameterGroups() const override;
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
