#include "../effect_registry.h"
#include "roughen_edges_effect.h"
#include <cmath>
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<RoughenEdgesEffect> s_reg("Roughen Edges", "Stylize");

RoughenEdgesEffect::RoughenEdgesEffect() {
    addParameter(EffectParameter::makeDropdown("edgeType", "Edge Type", {"Roughen", "Spiky", "Cut"}, 0));
    addParameter(EffectParameter::makeFloat("border", "Border", 0.0, 500.0, 20.0));
    addParameter(EffectParameter::makeFloat("edgeSharpness", "Edge Sharpness", 0.0, 10.0, 2.0));
    addParameter(EffectParameter::makeFloat("fractalInfluence", "Fractal Influence", 0.0, 100.0, 50.0));
}

std::unique_ptr<Effect> RoughenEdgesEffect::clone() const {
    auto e = std::make_unique<RoughenEdgesEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void RoughenEdgesEffect::render(PixelBuffer& buffer, double time) {
    if (buffer.width == 0 || buffer.height == 0) return;

    int edgeType = getDropdownParam("edgeType");
    double border = getFloatParam("border");
    double sharpness = getFloatParam("edgeSharpness");
    double influence = getFloatParam("fractalInfluence") / 100.0;

    auto hash = [](int x, int y) -> double {
        int h = x * 374761393 + y * 668265263;
        h = (h ^ (h >> 13)) * 1274126177;
        return static_cast<double>((h ^ (h >> 16)) & 0x7FFFFFFF) / 2147483647.0;
    };

    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);
    tmp.data = buffer.data;

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            double alpha = p[3] / 255.0;

            if (alpha < 0.01) continue;

            double edgeDist = border;
            for (int dy = -static_cast<int>(border); dy <= static_cast<int>(border); dy++) {
                for (int dx = -static_cast<int>(border); dx <= static_cast<int>(border); dx++) {
                    int sx = std::clamp(x + dx, 0, buffer.width - 1);
                    int sy = std::clamp(y + dy, 0, buffer.height - 1);
                    const uint8_t* sp = buffer.pixelAt(sx, sy);
                    if (sp[3] < 128) {
                        double d = std::sqrt(dx * dx + dy * dy);
                        edgeDist = std::min(edgeDist, d);
                    }
                }
            }

            double noise = hash(x + static_cast<int>(time * 10), y) * influence;
            double threshold = border * (1.0 + (edgeType == 1 ? noise : -noise));

            if (edgeDist < threshold) {
                double edgeAlpha = std::clamp((edgeDist / std::max(threshold, 0.1)), 0.0, 1.0);
                edgeAlpha = std::pow(edgeAlpha, 1.0 / std::max(sharpness, 0.01));
                uint8_t* dst = tmp.pixelAt(x, y);
                dst[3] = static_cast<uint8_t>(p[3] * edgeAlpha);
            }
        }
    }
    buffer.data = tmp.data;
}

} // namespace FreeEffect
