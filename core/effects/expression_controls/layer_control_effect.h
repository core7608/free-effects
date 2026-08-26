#pragma once
#include "../effect.h"

namespace FreeEffect {

class LayerControlEffect : public Effect {
public:
    LayerControlEffect();
    std::string getName() const override { return "Layer Control"; }
    std::string getCategory() const override { return "Expression Controls"; }
    std::string getDescription() const override { return "A layer reference control for expressions"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
