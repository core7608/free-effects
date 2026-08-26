#pragma once
#include "../effect.h"

namespace FreeEffect {

class LumaKeyEffect : public Effect {
public:
    LumaKeyEffect();
    std::string getName() const override { return "Luma Key"; }
    std::string getCategory() const override { return "Keying"; }
    std::vector<ParameterGroup> getParameterGroups() const override;
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
