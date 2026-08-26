#include "../effect_registry.h"
#include "depth_of_field_persp_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<DepthOfFieldPerspEffect> s_reg("Depth of Field (Perspective)", "Perspective");

DepthOfFieldPerspEffect::DepthOfFieldPerspEffect() {
    addParameter(EffectParameter::makeFloat("focalPlane", "Focal Plane", 0.0, 1000.0, 500.0));
    addParameter(EffectParameter::makeFloat("focalRange", "Focal Range", 0.0, 500.0, 100.0));
    addParameter(EffectParameter::makeInt("maxBlur", "Maximum Blur", 1, 50, 10));
}

std::unique_ptr<Effect> DepthOfFieldPerspEffect::clone() const {
    auto e = std::make_unique<DepthOfFieldPerspEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void DepthOfFieldPerspEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    float focal = getFloatParam("focalPlane");
    float range = getFloatParam("focalRange");
    int maxBlur = getIntParam("maxBlur");

    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);
    std::copy(buffer.data.begin(), buffer.data.end(), tmp.data.begin());

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            float depth = static_cast<float>(y) / buffer.height * 1000.0f;
            float blur = std::abs(depth - focal);
            blur = std::max(0.0f, blur - range);
            int radius = std::min(static_cast<int>(blur / 50.0f), maxBlur);
            if (radius <= 0) continue;

            float rS = 0, gS = 0, bS = 0, wS = 0;
            for (int dy = -radius; dy <= radius; dy++) {
                for (int dx = -radius; dx <= radius; dx++) {
                    int sx = std::clamp(x + dx, 0, buffer.width - 1);
                    int sy = std::clamp(y + dy, 0, buffer.height - 1);
                    const uint8_t* p = tmp.pixelAt(sx, sy);
                    float w = 1.0f;
                    rS += p[0]*w; gS += p[1]*w; bS += p[2]*w; wS += w;
                }
            }
            uint8_t* dst = buffer.pixelAt(x, y);
            dst[0] = static_cast<uint8_t>(std::clamp(rS/wS, 0.0f, 255.0f));
            dst[1] = static_cast<uint8_t>(std::clamp(gS/wS, 0.0f, 255.0f));
            dst[2] = static_cast<uint8_t>(std::clamp(bS/wS, 0.0f, 255.0f));
        }
    }
}

} // namespace FreeEffect
