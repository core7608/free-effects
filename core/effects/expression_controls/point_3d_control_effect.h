#pragma once
#include "../effect.h"

namespace FreeEffect {

class Point3DControlEffect : public Effect {
public:
    Point3DControlEffect();
    std::string getName() const override { return "3D Point Control"; }
    std::string getCategory() const override { return "Expression Controls"; }
    std::string getDescription() const override { return "A 3D point control for expressions"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
