#include "../effect_registry.h"
#include "lens_distortion_effect.h"
#include <cmath>
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<LensDistortionEffect> s_reg("Lens Distortion", "Distort");

LensDistortionEffect::LensDistortionEffect() {
    addParameter(EffectParameter::makeFloat("curvature", "Curvature", -100.0, 100.0, 0.0));
    addParameter(EffectParameter::makeFloat("horizontalPrism", "Horizontal Prism", -100.0, 100.0, 0.0));
    addParameter(EffectParameter::makeFloat("verticalPrism", "Vertical Prism", -100.0, 100.0, 0.0));
    addParameter(EffectParameter::makeColor("fillColor", "Fill Color", Color{0.0, 0.0, 0.0, 1.0}));
}

std::unique_ptr<Effect> LensDistortionEffect::clone() const {
    auto e = std::make_unique<LensDistortionEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void LensDistortionEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    if (buffer.width == 0 || buffer.height == 0) return;

    double curvature = getFloatParam("curvature") / 100.0;
    double hPrism = getFloatParam("horizontalPrism") / 100.0;
    double vPrism = getFloatParam("verticalPrism") / 100.0;
    Color fill = getColorParam("fillColor");

    double cx = buffer.width * 0.5;
    double cy = buffer.height * 0.5;
    double maxR = std::sqrt(cx * cx + cy * cy);

    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            double dx = (x - cx) / maxR;
            double dy = (y - cy) / maxR;

            double r2 = dx * dx + dy * dy;
            double k = 1.0 + curvature * r2;

            double srcX = cx + dx * k * maxR + hPrism * maxR * r2;
            double srcY = cy + dy * k * maxR + vPrism * maxR * r2;

            int sx = static_cast<int>(std::round(srcX));
            int sy = static_cast<int>(std::round(srcY));

            uint8_t* dst = tmp.pixelAt(x, y);
            if (sx >= 0 && sx < buffer.width && sy >= 0 && sy < buffer.height) {
                const uint8_t* src = buffer.pixelAt(sx, sy);
                dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3];
            } else {
                dst[0] = static_cast<uint8_t>(std::clamp(fill.r, 0.0, 255.0));
                dst[1] = static_cast<uint8_t>(std::clamp(fill.g, 0.0, 255.0));
                dst[2] = static_cast<uint8_t>(std::clamp(fill.b, 0.0, 255.0));
                dst[3] = static_cast<uint8_t>(std::clamp(fill.a * 255.0, 0.0, 255.0));
            }
        }
    }
    buffer.data = tmp.data;
}

} // namespace FreeEffect
