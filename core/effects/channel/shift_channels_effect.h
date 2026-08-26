#pragma once
#include "../effect.h"

namespace FreeEffect {

class ShiftChannelsEffect : public Effect {
public:
    ShiftChannelsEffect();
    std::string getName() const override { return "Shift Channels"; }
    std::string getCategory() const override { return "Channel"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
