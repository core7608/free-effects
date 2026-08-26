#pragma once
#include "../effect.h"

namespace FreeEffect {

class SetChannelsEffect : public Effect {
public:
    SetChannelsEffect();
    std::string getName() const override { return "Set Channels"; }
    std::string getCategory() const override { return "Channel"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
