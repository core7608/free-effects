#pragma once
#include "../effect.h"

namespace FreeEffect {

class KeyCleanerEffect : public Effect {
public:
    KeyCleanerEffect();
    std::string getName() const override { return "Key Cleaner"; }
    std::string getCategory() const override { return "Keying"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
