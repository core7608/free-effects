#pragma once

#include "../rendering/renderer.h"
#include "../timeline/composition.h"

namespace FreeEffect {

class MotionBlurRenderer {
public:
    MotionBlurRenderer();

    PixelBuffer renderWithMotionBlur(const Composition& comp, double time,
                                     double shutterAngle, int samples);

    void setRenderer(Renderer* renderer) { m_renderer = renderer; }

private:
    Renderer* m_renderer = nullptr;
};

} // namespace FreeEffect
