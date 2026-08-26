#pragma once
#include "../effect.h"

namespace FreeEffect {

class SpillSuppressorEffect : public Effect {
public:
    SpillSuppressorEffect();
    std::string getName() const override { return "Spill Suppressor"; }
    std::string getCategory() const override { return "Keying"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
