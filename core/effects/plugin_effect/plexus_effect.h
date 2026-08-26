#pragma once
#include "../effect.h"

namespace FreeEffect {

class PlexusEffect : public Effect {
public:
    PlexusEffect();
    std::string getName() const override { return "Plexus"; }
    std::string getCategory() const override { return "Plugin Effect"; }
    std::string getSubCategory() const override { return "Triptych"; }
    std::vector<ParameterGroup> getParameterGroups() const override;
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
