#include "../effect_registry.h"
#include "color_halftone_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<ColorHalftoneEffect> s_reg("Color Halftone", "Stylize");

ColorHalftoneEffect::ColorHalftoneEffect() {
    addParameter(EffectParameter::makeFloat("maxRadius", "Max Radius", 1.0, 50.0, 8.0));
    addParameter(EffectParameter::makeAngle("angle1", "Channel 1 Angle", 15.0));
    addParameter(EffectParameter::makeAngle("angle2", "Channel 2 Angle", 45.0));
    addParameter(EffectParameter::makeAngle("angle3", "Channel 3 Angle", 75.0));
    addParameter(EffectParameter::makeAngle("angle4", "Channel 4 Angle", 105.0));
}

std::unique_ptr<Effect> ColorHalftoneEffect::clone() const {
    auto e = std::make_unique<ColorHalftoneEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void ColorHalftoneEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    float maxR = getFloatParam("maxRadius");
    float a1 = getFloatParam("angle1") * 3.14159265f / 180.0f;
    float a2 = getFloatParam("angle2") * 3.14159265f / 180.0f;
    float a3 = getFloatParam("angle3") * 3.14159265f / 180.0f;

    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);
    std::copy(buffer.data.begin(), buffer.data.end(), tmp.data.begin());

    int cellSize = static_cast<int>(maxR * 2);
    for (int gy = 0; gy < buffer.height; gy += cellSize) {
        for (int gx = 0; gx < buffer.width; gx += cellSize) {
            int cx2 = gx + cellSize / 2, cy2 = gy + cellSize / 2;
            const uint8_t* p = tmp.pixelAt(std::min(cx2, buffer.width-1), std::min(cy2, buffer.height-1));
            float r = p[0] / 255.0f, g = p[1] / 255.0f, b = p[2] / 255.0f;
            float rR = (1.0f - r) * maxR;
            float rG = (1.0f - g) * maxR;
            float rB = (1.0f - b) * maxR;

            auto drawDot = [&](float cx3, float cy3, float rad, float angle, float val) {
                float dx = cx3 - std::cos(angle) * cellSize * 0.15f;
                float dy = cy3 - std::sin(angle) * cellSize * 0.15f;
                for (int dy2 = static_cast<int>(-rad); dy2 <= static_cast<int>(rad); dy2++) {
                    for (int dx2 = static_cast<int>(-rad); dx2 <= static_cast<int>(rad); dx2++) {
                        if (dx2*dx2+dy2*dy2 > rad*rad) continue;
                        int px = std::clamp(static_cast<int>(dx)+dx2, 0, buffer.width-1);
                        int py = std::clamp(static_cast<int>(dy)+dy2, 0, buffer.height-1);
                        uint8_t* dst = buffer.pixelAt(px, py);
                        dst[0] = 0; dst[1] = 0; dst[2] = 0;
                    }
                }
            };
            drawDot(cx2, cy2, rR, a1, r);
            drawDot(cx2, cy2, rG, a2, g);
            drawDot(cx2, cy2, rB, a3, b);
        }
    }
}

} // namespace FreeEffect
