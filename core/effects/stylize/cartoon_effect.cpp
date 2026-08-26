#include "../effect_registry.h"
#include "cartoon_effect.h"
#include <cmath>
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<CartoonEffect> s_reg("Cartoon", "Stylize");

CartoonEffect::CartoonEffect() {
    addParameter(EffectParameter::makeInt("levels", "Color Levels", 2, 20, 6));
    addParameter(EffectParameter::makeFloat("edge_threshold", "Edge Threshold", 0.0, 1.0, 0.2));
    addParameter(EffectParameter::makeFloat("edge_thickness", "Edge Thickness", 1.0, 5.0, 2.0));
    addParameter(EffectParameter::makeColor("edge_color", "Edge Color", Color{0.0, 0.0, 0.0, 1.0}));
}

std::vector<ParameterGroup> CartoonEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeInt("levels", "Color Levels", 2, 20, 6),
        EffectParameter::makeFloat("edge_threshold", "Edge Threshold", 0.0, 1.0, 0.2),
        EffectParameter::makeFloat("edge_thickness", "Edge Thickness", 1.0, 5.0, 2.0),
        EffectParameter::makeColor("edge_color", "Edge Color", Color{0.0, 0.0, 0.0, 1.0})
    }}};
}

std::unique_ptr<Effect> CartoonEffect::clone() const {
    auto e = std::make_unique<CartoonEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void CartoonEffect::render(PixelBuffer& buffer, double time) {
    if (buffer.width == 0 || buffer.height == 0) return;
    int levels = getIntParam("levels");
    double edgeThresh = getFloatParam("edge_threshold");
    double edgeThick = getFloatParam("edge_thickness");
    Color ec = getColorParam("edge_color");
    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);
    tmp.data = buffer.data;
    double step = 255.0 / levels;
    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            p[0] = static_cast<uint8_t>(std::round(p[0] / step) * step);
            p[1] = static_cast<uint8_t>(std::round(p[1] / step) * step);
            p[2] = static_cast<uint8_t>(std::round(p[2] / step) * step);
        }
    }
    int rad = static_cast<int>(edgeThick);
    for (int y = rad; y < buffer.height - rad; y++) {
        for (int x = rad; x < buffer.width - rad; x++) {
            const uint8_t* c = tmp.pixelAt(x, y);
            double lumC = (c[0] * 0.299 + c[1] * 0.587 + c[2] * 0.114) / 255.0;
            double maxDiff = 0.0;
            for (int dy = -rad; dy <= rad; dy++) {
                for (int dx = -rad; dx <= rad; dx++) {
                    if (dx == 0 && dy == 0) continue;
                    const uint8_t* n = tmp.pixelAt(x + dx, y + dy);
                    double lumN = (n[0] * 0.299 + n[1] * 0.587 + n[2] * 0.114) / 255.0;
                    maxDiff = std::max(maxDiff, std::abs(lumC - lumN));
                }
            }
            if (maxDiff > edgeThresh) {
                uint8_t* p = buffer.pixelAt(x, y);
                p[0] = static_cast<uint8_t>(ec.r * 255.0);
                p[1] = static_cast<uint8_t>(ec.g * 255.0);
                p[2] = static_cast<uint8_t>(ec.b * 255.0);
            }
        }
    }
}

} // namespace FreeEffect
