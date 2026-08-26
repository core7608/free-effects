#pragma once
#include "../effect.h"

namespace FreeEffect {

class RemoveColorMattingEffect : public Effect {
public:
    RemoveColorMattingEffect();
    std::string getName() const override { return "Remove Color Matting"; }
    std::string getCategory() const override { return "Channel"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
