#pragma once
#include "../effect.h"

namespace FreeEffect {

class CCSphereEffect : public Effect {
public:
    CCSphereEffect();
    std::string getName() const override { return "CC Sphere"; }
    std::string getCategory() const override { return "Perspective"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
