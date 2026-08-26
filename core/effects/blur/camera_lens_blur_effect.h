#pragma once
#include "../effect.h"

namespace FreeEffect {

class CameraLensBlurEffect : public Effect {
public:
    CameraLensBlurEffect();
    std::string getName() const override { return "Camera Lens Blur"; }
    std::string getCategory() const override { return "Blur & Sharpen"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
