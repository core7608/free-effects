#pragma once
#include "../effect.h"

namespace FreeEffect {

class MeshWarpEffect : public Effect {
public:
    MeshWarpEffect();
    std::string getName() const override { return "Mesh Warp"; }
    std::string getCategory() const override { return "Distort"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
