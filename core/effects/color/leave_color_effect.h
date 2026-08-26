#pragma once
#include "../effect.h"

namespace FreeEffect {

class LeaveColorEffect : public Effect {
public:
    LeaveColorEffect();
    std::string getName() const override { return "Leave Color"; }
    std::string getCategory() const override { return "Color Correction"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
