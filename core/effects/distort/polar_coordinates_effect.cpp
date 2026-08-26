#include "../effect_registry.h"
#include "polar_coordinates_effect.h"
#include <cmath>
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<PolarCoordinatesEffect> s_reg("Polar Coordinates", "Distort");

PolarCoordinatesEffect::PolarCoordinatesEffect() {
    addParameter(EffectParameter::makeFloat("interpolation", "Interpolation", 0.0, 1.0, 0.0));
    addParameter(EffectParameter::makeInt("conversion", "Conversion", 0, 1, 0));
}

std::vector<ParameterGroup> PolarCoordinatesEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeFloat("interpolation", "Interpolation", 0.0, 1.0, 0.0),
        EffectParameter::makeInt("conversion", "Conversion", 0, 1, 0)
    }}};
}

std::unique_ptr<Effect> PolarCoordinatesEffect::clone() const {
    auto e = std::make_unique<PolarCoordinatesEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void PolarCoordinatesEffect::render(PixelBuffer& buffer, double time) {
    if (buffer.width == 0 || buffer.height == 0) return;
    double interp = getFloatParam("interpolation");
    int conv = getIntParam("conversion");
    double cx = buffer.width / 2.0;
    double cy = buffer.height / 2.0;
    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);
    tmp.data = buffer.data;
    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            double dx = (x - cx) / cx;
            double dy = (y - cy) / cy;
            double r = std::sqrt(dx * dx + dy * dy);
            double theta = std::atan2(dy, dx);
            double rectX, rectY;
            if (conv == 0) {
                rectX = (theta / 3.14159 + 1.0) * 0.5;
                rectY = r;
            } else {
                rectX = std::cos(dy * 3.14159) * (1.0 - std::abs(dx));
                rectY = std::sin(dx * 3.14159) * (1.0 - std::abs(dy));
            }
            rectX = std::clamp(rectX * buffer.width, 0.0, static_cast<double>(buffer.width - 1));
            rectY = std::clamp(rectY * buffer.height, 0.0, static_cast<double>(buffer.height - 1));
            int sx = static_cast<int>(std::round(rectX));
            int sy = static_cast<int>(std::round(rectY));
            const uint8_t* src = tmp.pixelAt(sx, sy);
            uint8_t* dst = buffer.pixelAt(x, y);
            if (interp > 0.001) {
                int sx2 = std::clamp(static_cast<int>(std::round(x + (sx - x) * interp)), 0, buffer.width - 1);
                int sy2 = std::clamp(static_cast<int>(std::round(y + (sy - y) * interp)), 0, buffer.height - 1);
                const uint8_t* src2 = tmp.pixelAt(sx2, sy2);
                double t = interp;
                for (int c = 0; c < 4; c++) {
                    dst[c] = static_cast<uint8_t>(src[c] * t + tmp.pixelAt(x, y)[c] * (1.0 - t));
                }
            } else {
                dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3];
            }
        }
    }
}

} // namespace FreeEffect
