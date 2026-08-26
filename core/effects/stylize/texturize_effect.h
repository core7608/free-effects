#pragma once
#include "../effect.h"

namespace FreeEffect {

class TexturizeEffect : public Effect {
public:
    TexturizeEffect();
    std::string getName() const override { return "Texturize"; }
    std::string getCategory() const override { return "Stylize"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
