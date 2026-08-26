#pragma once
#include "../effect.h"

namespace FreeEffect {

class MatrixRainEffect : public Effect {
public:
    MatrixRainEffect();
    std::string getName() const override { return "Matrix Rain"; }
    std::string getCategory() const override { return "Simulation"; }
    std::vector<ParameterGroup> getParameterGroups() const override;
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
