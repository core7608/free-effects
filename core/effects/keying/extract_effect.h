#pragma once
#include "../effect.h"

namespace FreeEffect {

class ExtractEffect : public Effect {
public:
    ExtractEffect();
    std::string getName() const override { return "Extract"; }
    std::string getCategory() const override { return "Keying"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
