#pragma once
#include "../effect.h"

namespace FreeEffect {

class ExpandEffect : public Effect {
public:
    ExpandEffect();
    std::string getName() const override { return "Expand"; }
    std::string getCategory() const override { return "Utility"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
