#pragma once
#include "../effect.h"

namespace FreeEffect {

class SaberEffect : public Effect {
public:
    SaberEffect();
    std::string getName() const override { return "Saber"; }
    std::string getCategory() const override { return "Plugin Effect"; }
    std::string getSubCategory() const override { return "Video Copilot"; }
    std::vector<ParameterGroup> getParameterGroups() const override;
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
