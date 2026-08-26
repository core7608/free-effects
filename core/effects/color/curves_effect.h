#pragma once
#include "../effect.h"
#include <array>

namespace FreeEffect {

class CurvesEffect : public Effect {
public:
    CurvesEffect();
    std::string getName() const override { return "Curves"; }
    std::string getCategory() const override { return "Color Correction"; }
    std::vector<ParameterGroup> getParameterGroups() const override;
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;

private:
    void buildLUT(int channel);
    std::array<std::array<uint8_t, 256>, 4> m_lut;
};

} // namespace FreeEffect
