#include "motion_blur.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

MotionBlurRenderer::MotionBlurRenderer() {
}

PixelBuffer MotionBlurRenderer::renderWithMotionBlur(const Composition& comp, double time,
                                                     double shutterAngle, int samples) {
    if (samples < 1) samples = 1;

    double fps = comp.getFrameRateValue();
    if (fps <= 0.0) fps = 30.0;
    double frameDuration = 1.0 / fps;

    double halfRange = (shutterAngle / 360.0) * frameDuration;
    double timeStart = time - halfRange;
    double timeEnd = time + halfRange;
    double timeStep = 0.0;
    if (samples > 1) {
        timeStep = (timeEnd - timeStart) / static_cast<double>(samples - 1);
    }

    int width = comp.getResolution().width;
    int height = comp.getResolution().height;
    int pixelCount = width * height;

    PixelBuffer accum;
    accum.resize(width, height);

    Renderer renderer;
    renderer.setResolution(width, height);

    double weight = 1.0 / static_cast<double>(samples);

    for (int i = 0; i < samples; ++i) {
        double sampleTime = timeStart + timeStep * i;
        sampleTime = std::clamp(sampleTime, 0.0, comp.getDuration());

        PixelBuffer sample = renderer.renderFrame(comp, sampleTime);

        for (int p = 0; p < pixelCount; ++p) {
            int offset = p * 4;
            double srcA = sample.data[offset + 3] / 255.0 * weight;
            double dstA = accum.data[offset + 3] / 255.0;
            double outA = srcA + dstA * (1.0 - srcA);

            if (outA > 1e-7) {
                accum.data[offset]     = static_cast<uint8_t>(
                    std::clamp((sample.data[offset]     * srcA + accum.data[offset]     * dstA * (1.0 - srcA)) / outA, 0.0, 255.0));
                accum.data[offset + 1] = static_cast<uint8_t>(
                    std::clamp((sample.data[offset + 1] * srcA + accum.data[offset + 1] * dstA * (1.0 - srcA)) / outA, 0.0, 255.0));
                accum.data[offset + 2] = static_cast<uint8_t>(
                    std::clamp((sample.data[offset + 2] * srcA + accum.data[offset + 2] * dstA * (1.0 - srcA)) / outA, 0.0, 255.0));
                accum.data[offset + 3] = static_cast<uint8_t>(
                    std::clamp(outA * 255.0, 0.0, 255.0));
            }
        }
    }

    return accum;
}

} // namespace FreeEffect
