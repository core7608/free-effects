#pragma once
#include "../effect.h"

namespace FreeEffect {

class RefineHardMatteEffect : public Effect {
public:
    RefineHardMatteEffect();
    std::string getName() const override { return "Refine Hard Matte"; }
    std::string getCategory() const override { return "Keying"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
