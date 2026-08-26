#pragma once
#include "../effect.h"

namespace FreeEffect {

class ParticularEffect : public Effect {
public:
    ParticularEffect();
    std::string getName() const override { return "Particular"; }
    std::string getCategory() const override { return "Plugin Effect"; }
    std::string getSubCategory() const override { return "Trapcode"; }
    std::vector<ParameterGroup> getParameterGroups() const override;
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
