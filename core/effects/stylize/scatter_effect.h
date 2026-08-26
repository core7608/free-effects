#pragma once
#include "../effect.h"

namespace FreeEffect {

class ScatterEffect : public Effect {
public:
    ScatterEffect();
    std::string getName() const override { return "Scatter"; }
    std::string getCategory() const override { return "Stylize"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
