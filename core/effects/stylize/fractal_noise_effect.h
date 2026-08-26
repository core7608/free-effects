#pragma once
#include "../effect.h"

namespace FreeEffect {

class FractalNoiseEffect : public Effect {
public:
    FractalNoiseEffect();
    std::string getName() const override { return "Fractal Noise"; }
    std::string getCategory() const override { return "Stylize"; }
    std::vector<ParameterGroup> getParameterGroups() const override;
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;

private:
    float noise2D(float x, float y) const;
    float fractalNoise(float x, float y, int octaves, float lacunarity, float gain) const;
};

} // namespace FreeEffect
