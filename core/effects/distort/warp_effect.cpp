#include "../effect_registry.h"
#include "warp_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<WarpEffect> s_reg("Warp", "Distort");

WarpEffect::WarpEffect() {
    addParameter(EffectParameter::makeDropdown("warpStyle", "Warp Style",
        {"Arc", "Arc Lower", "Arc Upper", "Arch", "Bulge", "Bulge Upper", "Bulge Lower",
         "Flag", "Fish", "Rise", "Fish Eye", "Fisheye", "Tube", "Tube Vertical"}, 0));
    addParameter(EffectParameter::makeFloat("bend", "Bend", -100.0, 100.0, 50.0));
    addParameter(EffectParameter::makeVec2("anchor", "Anchor", {0.5, 0.5}));
    addParameter(EffectParameter::makeFloat("horizontalDistortion", "Horizontal Distortion", -100.0, 100.0, 0.0));
}

std::unique_ptr<Effect> WarpEffect::clone() const {
    auto e = std::make_unique<WarpEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void WarpEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    int style = getDropdownParam("warpStyle");
    float bend = getFloatParam("bend") / 100.0f;
    Vec2 anchor = getVec2Param("anchor");
    float horizDist = getFloatParam("horizontalDistortion") / 100.0f;

    float ax = anchor.x * buffer.width;
    float ay = anchor.y * buffer.height;

    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);
    std::copy(buffer.data.begin(), buffer.data.end(), tmp.data.begin());

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            float nx = (x - ax) / (buffer.width * 0.5f);
            float ny = (y - ay) / (buffer.height * 0.5f);
            float srcX = x, srcY = y;

            switch (style) {
                case 0: case 1: case 2: { // Arc variants
                    float arcAmount = bend * ny * ny;
                    srcX = x + nx * arcAmount * buffer.width * 0.5f;
                    srcY = y + bend * (1.0f - nx * nx) * buffer.height * 0.3f;
                    break;
                }
                case 3: { // Arch
                    srcY = y + bend * (1.0f - nx * nx) * buffer.height * 0.4f;
                    break;
                }
                case 4: { // Bulge
                    float d = nx * nx + ny * ny;
                    float scale = 1.0f + bend * (1.0f - d);
                    srcX = ax + (x - ax) * scale;
                    srcY = ay + (y - ay) * scale;
                    break;
                }
                case 5: case 6: { // Bulge Upper/Lower
                    float nyAdj = (style == 5) ? std::min(ny, 0.0f) : std::max(ny, 0.0f);
                    float d = nx * nx + nyAdj * nyAdj;
                    float scale = 1.0f + bend * (1.0f - d);
                    srcX = ax + (x - ax) * scale;
                    srcY = ay + (y - ay) * scale;
                    break;
                }
                case 7: { // Flag
                    srcX = x + bend * std::sin(ny * 3.14159265f * 2.0f) * buffer.width * 0.1f;
                    srcY = y + bend * std::cos(nx * 3.14159265f) * buffer.height * 0.05f;
                    break;
                }
                case 8: case 10: case 11: { // Fish / Fisheye
                    float d = nx * nx + ny * ny;
                    float r = std::sqrt(d);
                    float theta = std::atan2(ny, nx);
                    float newR = std::pow(r, 1.0f + bend * 0.5f);
                    srcX = ax + std::cos(theta) * newR * buffer.width * 0.5f;
                    srcY = ay + std::sin(theta) * newR * buffer.height * 0.5f;
                    break;
                }
                case 9: { // Rise
                    srcY = y - bend * (1.0f - nx * nx) * buffer.height * 0.3f;
                    break;
                }
                case 12: case 13: { // Tube
                    srcX = x + bend * nx * buffer.width * 0.2f;
                    break;
                }
            }

            srcX += horizDist * ny * buffer.width * 0.2f;
            int sx = std::clamp(static_cast<int>(srcX + 0.5f), 0, buffer.width - 1);
            int sy = std::clamp(static_cast<int>(srcY + 0.5f), 0, buffer.height - 1);
            const uint8_t* p = tmp.pixelAt(sx, sy);
            uint8_t* dst = buffer.pixelAt(x, y);
            dst[0] = p[0]; dst[1] = p[1]; dst[2] = p[2]; dst[3] = p[3];
        }
    }
}

} // namespace FreeEffect
