#include "../effect_registry.h"
#include "cc_lens_effect.h"
#include <cmath>
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<CCLensEffect> s_reg("CC Lens", "Distort");

CCLensEffect::CCLensEffect() {
    addParameter(EffectParameter::makeFloat("size", "Size", 0.0, 500.0, 100.0));
    addParameter(EffectParameter::makeFloat("convergence", "Convergence", -1000.0, 1000.0, 500.0));
    addParameter(EffectParameter::makeVec2("center", "Center", {0.5, 0.5}));
}

std::unique_ptr<Effect> CCLensEffect::clone() const {
    auto e = std::make_unique<CCLensEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void CCLensEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    if (buffer.width == 0 || buffer.height == 0) return;

    double size = getFloatParam("size");
    double convergence = getFloatParam("convergence");
    Vec2 ctr = getVec2Param("center");
    double cx = ctr.x * buffer.width;
    double cy = ctr.y * buffer.height;

    double maxR = std::max(buffer.width, buffer.height) * 0.5;
    double radius = size * 0.01 * maxR;

    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            double dx = x - cx;
            double dy = y - cy;
            double dist = std::sqrt(dx * dx + dy * dy);

            double srcX, srcY;
            if (dist < radius && radius > 0) {
                double norm = dist / radius;
                double z = std::sqrt(std::max(1.0 - norm * norm, 0.0));
                double displacement = z * convergence / std::max(radius, 1.0);
                srcX = cx + dx * displacement;
                srcY = cy + dy * displacement;
            } else {
                srcX = x;
                srcY = y;
            }

            int sx = std::clamp(static_cast<int>(std::round(srcX)), 0, buffer.width - 1);
            int sy = std::clamp(static_cast<int>(std::round(srcY)), 0, buffer.height - 1);

            const uint8_t* src = buffer.pixelAt(sx, sy);
            uint8_t* dst = tmp.pixelAt(x, y);
            dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3];
        }
    }
    buffer.data = tmp.data;
}

} // namespace FreeEffect
