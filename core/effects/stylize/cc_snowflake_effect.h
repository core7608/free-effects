#pragma once
#include "../effect.h"

namespace FreeEffect {

class CCSnowflakeEffect : public Effect {
public:
    CCSnowflakeEffect();
    std::string getName() const override { return "CC Snowflake"; }
    std::string getCategory() const override { return "Stylize"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
