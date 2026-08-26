#pragma once
#include "../effect.h"

namespace FreeEffect {

class OilPaintEffect : public Effect {
public:
    OilPaintEffect();
    std::string getName() const override { return "Oil Paint"; }
    std::string getCategory() const override { return "Stylize"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
