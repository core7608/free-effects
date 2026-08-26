#include "../effect_registry.h"
#include "write_on_effect.h"
#include <cmath>
#include <vector>
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<WriteOnEffect> s_reg("Write-On", "Generate");

WriteOnEffect::WriteOnEffect() {
    addParameter(EffectParameter::makeFloat("brush_size", "Brush Size", 1.0, 50.0, 5.0));
    addParameter(EffectParameter::makeFloat("brush_opacity", "Brush Opacity", 0.0, 1.0, 1.0));
    addParameter(EffectParameter::makeFloat("brush_hardness", "Brush Hardness", 0.0, 1.0, 0.8));
    addParameter(EffectParameter::makeColor("color", "Color", Color{1.0, 1.0, 1.0, 1.0}));
    addParameter(EffectParameter::makeFloat("spacing", "Spacing", 0.1, 5.0, 1.0));
    addParameter(EffectParameter::makeFloat("fade_out", "Fade Out", 0.0, 10.0, 2.0));
}

std::vector<ParameterGroup> WriteOnEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeFloat("brush_size", "Brush Size", 1.0, 50.0, 5.0),
        EffectParameter::makeFloat("brush_opacity", "Brush Opacity", 0.0, 1.0, 1.0),
        EffectParameter::makeFloat("brush_hardness", "Brush Hardness", 0.0, 1.0, 0.8),
        EffectParameter::makeColor("color", "Color", Color{1.0, 1.0, 1.0, 1.0}),
        EffectParameter::makeFloat("spacing", "Spacing", 0.1, 5.0, 1.0),
        EffectParameter::makeFloat("fade_out", "Fade Out", 0.0, 10.0, 2.0)
    }}};
}

std::unique_ptr<Effect> WriteOnEffect::clone() const {
    auto e = std::make_unique<WriteOnEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void WriteOnEffect::render(PixelBuffer& buffer, double time) {
    if (buffer.width == 0 || buffer.height == 0) return;
    double brushSize = getFloatParam("brush_size");
    double opacity = getFloatParam("brush_opacity");
    double hardness = getFloatParam("brush_hardness");
    Color wc = getColorParam("color");
    double spacing = getFloatParam("spacing");
    double fadeOut = getFloatParam("fade_out");
    int numStrokes = static_cast<int>(time * 5.0 / spacing) + 1;
    int maxStrokes = static_cast<int>(100.0 / spacing);
    numStrokes = std::min(numStrokes, maxStrokes);
    for (int s = 0; s < numStrokes; s++) {
        double st = s * spacing * 0.2;
        double age = time - st;
        if (age < 0 || (fadeOut > 0 && age > fadeOut)) continue;
        double seed1 = std::sin(s * 127.1 + 311.7) * 43758.5453;
        double seed2 = std::sin(s * 269.5 + 183.3) * 43758.5453;
        double f1 = seed1 - std::floor(seed1);
        double f2 = seed2 - std::floor(seed2);
        double px = f1 * buffer.width;
        double py = f2 * buffer.height;
        double fade = fadeOut > 0 ? 1.0 - age / fadeOut : 1.0;
        double a = opacity * fade;
        int radius = static_cast<int>(brushSize);
        for (int dy = -radius; dy <= radius; dy++) {
            for (int dx = -radius; dx <= radius; dx++) {
                int sx = static_cast<int>(std::round(px + dx));
                int sy = static_cast<int>(std::round(py + dy));
                if (sx < 0 || sx >= buffer.width || sy < 0 || sy >= buffer.height) continue;
                double dist = std::sqrt(dx * dx + dy * dy);
                if (dist > radius) continue;
                double edge = 1.0 - dist / (radius + 0.5);
                double brushAlpha = a * std::pow(edge, 1.0 / (hardness * 0.9 + 0.1));
                uint8_t* p = buffer.pixelAt(sx, sy);
                double sa = p[3] / 255.0;
                double outA = sa + brushAlpha * (1.0 - sa);
                if (outA > 0.001) {
                    p[0] = static_cast<uint8_t>((p[0] * sa + wc.r * 255.0 * brushAlpha * (1.0 - sa)) / outA);
                    p[1] = static_cast<uint8_t>((p[1] * sa + wc.g * 255.0 * brushAlpha * (1.0 - sa)) / outA);
                    p[2] = static_cast<uint8_t>((p[2] * sa + wc.b * 255.0 * brushAlpha * (1.0 - sa)) / outA);
                }
                p[3] = static_cast<uint8_t>(std::clamp(outA * 255.0, 0.0, 255.0));
            }
        }
    }
}

} // namespace FreeEffect
