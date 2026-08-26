#pragma once
#include "../effect.h"

namespace FreeEffect {

class Fog3DEffect : public Effect {
public:
    Fog3DEffect();
    std::string getName() const override { return "Fog 3D"; }
    std::string getCategory() const override { return "3D Channel"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
