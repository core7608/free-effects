#pragma once
#include "../effect.h"

namespace FreeEffect {

class Basic3DEffect : public Effect {
public:
    Basic3DEffect();
    std::string getName() const override { return "Basic 3D"; }
    std::string getCategory() const override { return "Perspective"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
