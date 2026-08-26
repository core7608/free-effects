#pragma once
#include "../effect.h"

namespace FreeEffect {

class DepthOfFieldPerspEffect : public Effect {
public:
    DepthOfFieldPerspEffect();
    std::string getName() const override { return "Depth of Field (Perspective)"; }
    std::string getCategory() const override { return "Perspective"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
