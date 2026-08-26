#include "../effect_registry.h"
#include "form_effect.h"
#include <cmath>
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<FormEffect> s_reg("Form", "Plugin Effect", "Trapcode");

FormEffect::FormEffect() {
    addParameter(EffectParameter::makeInt("grid_x", "Grid X", 2, 100, 20));
    addParameter(EffectParameter::makeInt("grid_y", "Grid Y", 2, 100, 20));
    addParameter(EffectParameter::makeFloat("amplitude", "Amplitude", 0.0, 100.0, 20.0));
    addParameter(EffectParameter::makeFloat("frequency", "Frequency", 0.01, 2.0, 0.5));
    addParameter(EffectParameter::makeFloat("speed", "Speed", 0.0, 5.0, 1.0));
    addParameter(EffectParameter::makeColor("color", "Color", Color{0.3, 0.6, 1.0, 0.8}));
    addParameter(EffectParameter::makeFloat("point_size", "Point Size", 1.0, 10.0, 2.0));
    addParameter(EffectParameter::makeBool("fade_edges", "Fade Edges", true));
}

std::vector<ParameterGroup> FormEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeInt("grid_x", "Grid X", 2, 100, 20),
        EffectParameter::makeInt("grid_y", "Grid Y", 2, 100, 20),
        EffectParameter::makeFloat("amplitude", "Amplitude", 0.0, 100.0, 20.0),
        EffectParameter::makeFloat("frequency", "Frequency", 0.01, 2.0, 0.5),
        EffectParameter::makeFloat("speed", "Speed", 0.0, 5.0, 1.0),
        EffectParameter::makeColor("color", "Color", Color{0.3, 0.6, 1.0, 0.8}),
        EffectParameter::makeFloat("point_size", "Point Size", 1.0, 10.0, 2.0),
        EffectParameter::makeBool("fade_edges", "Fade Edges", true)
    }}};
}

std::unique_ptr<Effect> FormEffect::clone() const {
    auto e = std::make_unique<FormEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void FormEffect::render(PixelBuffer& buffer, double time) {
    if (buffer.width == 0 || buffer.height == 0) return;
    int gridX = getIntParam("grid_x");
    int gridY = getIntParam("grid_y");
    double amp = getFloatParam("amplitude");
    double freq = getFloatParam("frequency");
    double spd = getFloatParam("speed");
    Color fc = getColorParam("color");
    double ptSz = getFloatParam("point_size");
    bool fadeEdges = getBoolParam("fade_edges");
    double cr = fc.r * 255.0;
    double cg = fc.g * 255.0;
    double cb = fc.b * 255.0;
    int ir = static_cast<int>(std::ceil(ptSz));
    for (int gy = 0; gy < gridY; gy++) {
        for (int gx = 0; gx < gridX; gx++) {
            double u = static_cast<double>(gx) / (gridX - 1);
            double v = static_cast<double>(gy) / (gridY - 1);
            double baseX = u * buffer.width;
            double baseY = v * buffer.height;
            double dispX = amp * std::sin(u * freq * 6.28318 + time * spd) * std::cos(v * freq * 3.14159 + time * spd * 0.7);
            double dispY = amp * std::cos(u * freq * 6.28318 - time * spd * 0.8) * std::sin(v * freq * 3.14159 + time * spd * 0.5);
            double px = baseX + dispX;
            double py = baseY + dispY;
            double edgeFade = 1.0;
            if (fadeEdges) {
                double ex = std::min(u, 1.0 - u) * 4.0;
                double ey = std::min(v, 1.0 - v) * 4.0;
                edgeFade = std::clamp(std::min(ex, ey), 0.0, 1.0);
            }
            int ipx = static_cast<int>(std::round(px));
            int ipy = static_cast<int>(std::round(py));
            for (int dy = -ir; dy <= ir; dy++) {
                for (int dx = -ir; dx <= ir; dx++) {
                    int sx = ipx + dx;
                    int sy = ipy + dy;
                    if (sx < 0 || sx >= buffer.width || sy < 0 || sy >= buffer.height) continue;
                    double dist = std::sqrt(dx * dx + dy * dy);
                    if (dist > ir) continue;
                    double falloff = 1.0 - dist / (ir + 0.5);
                    double fa = fc.a * falloff * edgeFade;
                    if (fa < 0.01) continue;
                    uint8_t* p = buffer.pixelAt(sx, sy);
                    double sa = p[3] / 255.0;
                    double outA = sa + fa * (1.0 - sa);
                    if (outA > 0.001) {
                        p[0] = static_cast<uint8_t>((p[0] * sa + cr * fa * (1.0 - sa)) / outA);
                        p[1] = static_cast<uint8_t>((p[1] * sa + cg * fa * (1.0 - sa)) / outA);
                        p[2] = static_cast<uint8_t>((p[2] * sa + cb * fa * (1.0 - sa)) / outA);
                    }
                    p[3] = static_cast<uint8_t>(std::clamp(outA * 255.0, 0.0, 255.0));
                }
            }
        }
    }
}

} // namespace FreeEffect
