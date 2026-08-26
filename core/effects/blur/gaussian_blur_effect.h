#pragma once
#include "../effect.h"

namespace FreeEffect {

class GaussianBlurEffect : public Effect {
public:
    GaussianBlurEffect();
    std::string getName() const override { return "Gaussian Blur"; }
    std::string getCategory() const override { return "Blur & Sharpen"; }
    std::vector<ParameterGroup> getParameterGroups() const override;
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;

private:
    void gaussianBlur(PixelBuffer& buf, int radius);
    void blurLine(uint8_t* line, int len, const float* kernel, int ksize);
};

} // namespace FreeEffect
