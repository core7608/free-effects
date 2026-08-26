#include "../effect_registry.h"
#include "fast_box_blur_effect.h"
#include <algorithm>
#include <vector>

namespace FreeEffect {

static EffectRegistrar<FastBoxBlurEffect> s_reg("Fast Box Blur", "Blur & Sharpen");

FastBoxBlurEffect::FastBoxBlurEffect() {
    addParameter(EffectParameter::makeInt("iterations", "Iterations", 1, 5, 2));
    addParameter(EffectParameter::makeInt("blurRadius", "Blur Radius", 1, 200, 3));
    addParameter(EffectParameter::makeBool("repeatEdgePixels", "Repeat Edge Pixels", true));
}

std::unique_ptr<Effect> FastBoxBlurEffect::clone() const {
    auto e = std::make_unique<FastBoxBlurEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void FastBoxBlurEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    int iterations = std::clamp(getIntParam("iterations"), 1, 5);
    int radius = getIntParam("blurRadius");
    bool repeatEdge = getBoolParam("repeatEdgePixels");
    if (radius <= 0) return;

    for (int iter = 0; iter < iterations; iter++) {
        PixelBuffer tmp;
        tmp.resize(buffer.width, buffer.height);

        for (int y = 0; y < buffer.height; y++) {
            float rAccum = 0, gAccum = 0, bAccum = 0, aAccum = 0;
            int count = 0;

            for (int k = -radius; k <= radius; k++) {
                int sx = repeatEdge ? std::clamp(k, 0, buffer.width - 1) : std::clamp(k, 0, buffer.width - 1);
                if (!repeatEdge && (k < 0 || k >= buffer.width)) continue;
                const uint8_t* p = buffer.pixelAt(sx, y);
                rAccum += p[0]; gAccum += p[1]; bAccum += p[2]; aAccum += p[3];
                count++;
            }

            for (int x = 0; x < buffer.width; x++) {
                uint8_t* dst = tmp.pixelAt(x, y);
                int c = std::max(count, 1);
                dst[0] = static_cast<uint8_t>(std::clamp(rAccum / c, 0.0f, 255.0f));
                dst[1] = static_cast<uint8_t>(std::clamp(gAccum / c, 0.0f, 255.0f));
                dst[2] = static_cast<uint8_t>(std::clamp(bAccum / c, 0.0f, 255.0f));
                dst[3] = static_cast<uint8_t>(std::clamp(aAccum / c, 0.0f, 255.0f));

                int removeX = x - radius;
                int addX = x + radius + 1;
                if (repeatEdge || (removeX >= 0 && removeX < buffer.width)) {
                    const uint8_t* rp = buffer.pixelAt(std::clamp(removeX, 0, buffer.width - 1), y);
                    rAccum -= rp[0]; gAccum -= rp[1]; bAccum -= rp[2]; aAccum -= rp[3];
                }
                if (repeatEdge || (addX >= 0 && addX < buffer.width)) {
                    const uint8_t* ap = buffer.pixelAt(std::clamp(addX, 0, buffer.width - 1), y);
                    rAccum += ap[0]; gAccum += ap[1]; bAccum += ap[2]; aAccum += ap[3];
                }
            }
        }

        for (int x = 0; x < buffer.width; x++) {
            float rAccum = 0, gAccum = 0, bAccum = 0, aAccum = 0;
            int count = 0;

            for (int k = -radius; k <= radius; k++) {
                int sy = repeatEdge ? std::clamp(k, 0, buffer.height - 1) : std::clamp(k, 0, buffer.height - 1);
                if (!repeatEdge && (k < 0 || k >= buffer.height)) continue;
                const uint8_t* p = tmp.pixelAt(x, sy);
                rAccum += p[0]; gAccum += p[1]; bAccum += p[2]; aAccum += p[3];
                count++;
            }

            for (int y = 0; y < buffer.height; y++) {
                uint8_t* dst = buffer.pixelAt(x, y);
                int c = std::max(count, 1);
                dst[0] = static_cast<uint8_t>(std::clamp(rAccum / c, 0.0f, 255.0f));
                dst[1] = static_cast<uint8_t>(std::clamp(gAccum / c, 0.0f, 255.0f));
                dst[2] = static_cast<uint8_t>(std::clamp(bAccum / c, 0.0f, 255.0f));
                dst[3] = static_cast<uint8_t>(std::clamp(aAccum / c, 0.0f, 255.0f));

                int removeY = y - radius;
                int addY = y + radius + 1;
                if (repeatEdge || (removeY >= 0 && removeY < buffer.height)) {
                    const uint8_t* rp = tmp.pixelAt(x, std::clamp(removeY, 0, buffer.height - 1));
                    rAccum -= rp[0]; gAccum -= rp[1]; bAccum -= rp[2]; aAccum -= rp[3];
                }
                if (repeatEdge || (addY >= 0 && addY < buffer.height)) {
                    const uint8_t* ap = tmp.pixelAt(x, std::clamp(addY, 0, buffer.height - 1));
                    rAccum += ap[0]; gAccum += ap[1]; bAccum += ap[2]; aAccum += ap[3];
                }
            }
        }
    }
}

} // namespace FreeEffect
