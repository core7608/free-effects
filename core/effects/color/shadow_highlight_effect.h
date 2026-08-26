#pragma once
#include "../effect.h"

namespace FreeEffect {

class ShadowHighlightEffect : public Effect {
public:
    ShadowHighlightEffect();
    std::string getName() const override { return "Shadow/Highlight"; }
    std::string getCategory() const override { return "Color"; }
    std::vector<ParameterGroup> getParameterGroups() const override;
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
