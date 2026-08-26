#pragma once
#include "../effect.h"

namespace FreeEffect {

class TurbulentDisplaceEffect : public Effect {
public:
    TurbulentDisplaceEffect();
    std::string getName() const override { return "Turbulent Displace"; }
    std::string getCategory() const override { return "Distort"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;

private:
    double noise2d(double x, double y) const;
    double fractalNoise(double x, double y, int octaves) const;
};

} // namespace FreeEffect
