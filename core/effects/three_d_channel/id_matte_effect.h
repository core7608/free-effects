#pragma once
#include "../effect.h"

namespace FreeEffect {

class IDMatteEffect : public Effect {
public:
    IDMatteEffect();
    std::string getName() const override { return "ID Matte"; }
    std::string getCategory() const override { return "3D Channel"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
