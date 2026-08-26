#pragma once
#include "../effect.h"

namespace FreeEffect {

class LightningEffect : public Effect {
public:
    LightningEffect();
    std::string getName() const override { return "Lightning"; }
    std::string getCategory() const override { return "Simulation"; }
    std::vector<ParameterGroup> getParameterGroups() const override;
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;

private:
    void drawBolt(PixelBuffer& buffer, int x0, int y0, int x1, int y1,
                  double thickness, double r, double g, double b, double a, int depth);
};

} // namespace FreeEffect
