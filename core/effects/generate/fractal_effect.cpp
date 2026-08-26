#include "../effect_registry.h"
#include "fractal_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<FractalEffect> s_reg("Fractal", "Generate");

FractalEffect::FractalEffect() {
    addParameter(EffectParameter::makeDropdown("type", "Type", {"Mandelbrot", "Julia", "Burning Ship", "Sierpinski"}, 0));
    addParameter(EffectParameter::makeInt("maxIterations", "Max Iterations", 1, 500, 100));
    addParameter(EffectParameter::makeFloat("zoom", "Zoom", 0.1, 100.0, 1.0));
    addParameter(EffectParameter::makeVec2("center", "Center", {-0.5, 0.0}));
    addParameter(EffectParameter::makeColor("color1", "Color 1", {0.0, 0.0, 0.0, 1.0}));
    addParameter(EffectParameter::makeColor("color2", "Color 2", {255.0, 255.0, 255.0, 1.0}));
}

std::unique_ptr<Effect> FractalEffect::clone() const {
    auto e = std::make_unique<FractalEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void FractalEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    int type = getDropdownParam("type");
    int maxIter = getIntParam("maxIterations");
    float zoom = getFloatParam("zoom");
    Vec2 center = getVec2Param("center");
    Color c1 = getColorParam("color1"), c2 = getColorParam("color2");

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            float zx = (static_cast<float>(x) / buffer.width - 0.5f) * 4.0f / zoom + center.x;
            float zy = (static_cast<float>(y) / buffer.height - 0.5f) * 4.0f / zoom + center.y;
            float cx2 = zx, cy2 = zy;
            if (type == 1) { cx2 = 0.285f; cy2 = 0.01f; }
            int iter = 0;
            while (zx * zx + zy * zy < 4.0f && iter < maxIter) {
                float tmp2 = zx * zx - zy * zy + cx2;
                if (type == 2) { zy = std::abs(2.0f * zx * zy) + cy2; zx = std::abs(tmp2); }
                else { zy = 2.0f * zx * zy + cy2; zx = tmp2; }
                iter++;
            }
            float t = static_cast<float>(iter) / maxIter;
            uint8_t* p = buffer.pixelAt(x, y);
            p[0] = static_cast<uint8_t>(c1.r + (c2.r - c1.r) * t);
            p[1] = static_cast<uint8_t>(c1.g + (c2.g - c1.g) * t);
            p[2] = static_cast<uint8_t>(c1.b + (c2.b - c1.b) * t);
            p[3] = 255;
        }
    }
}

} // namespace FreeEffect
