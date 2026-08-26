#pragma once
#include "../effect.h"
#include <random>

namespace FreeEffect {

class CellPatternEffect : public Effect {
public:
    CellPatternEffect();
    std::string getName() const override { return "Cell Pattern"; }
    std::string getCategory() const override { return "Generate"; }
    std::vector<ParameterGroup> getParameterGroups() const override;
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
