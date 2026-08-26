#include "../effect_registry.h"
#include "mir_effect.h"
#include <cmath>
#include <vector>
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<MirEffect> s_reg("Mir", "Plugin Effect", "Trapcode");

MirEffect::MirEffect() {
    addParameter(EffectParameter::makeInt("vertices_x", "Vertices X", 4, 100, 20));
    addParameter(EffectParameter::makeInt("vertices_y", "Vertices Y", 4, 100, 20));
    addParameter(EffectParameter::makeFloat("height", "Height", 0.0, 200.0, 50.0));
    addParameter(EffectParameter::makeFloat("frequency", "Frequency", 0.01, 3.0, 0.5));
    addParameter(EffectParameter::makeFloat("speed", "Speed", 0.0, 5.0, 1.0));
    addParameter(EffectParameter::makeColor("color", "Color", Color{0.0, 0.5, 1.0, 0.8}));
    addParameter(EffectParameter::makeBool("wireframe", "Wireframe", true));
    addParameter(EffectParameter::makeAngle("rotation_y", "Rotation Y", 0.0));
}

std::vector<ParameterGroup> MirEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeInt("vertices_x", "Vertices X", 4, 100, 20),
        EffectParameter::makeInt("vertices_y", "Vertices Y", 4, 100, 20),
        EffectParameter::makeFloat("height", "Height", 0.0, 200.0, 50.0),
        EffectParameter::makeFloat("frequency", "Frequency", 0.01, 3.0, 0.5),
        EffectParameter::makeFloat("speed", "Speed", 0.0, 5.0, 1.0),
        EffectParameter::makeColor("color", "Color", Color{0.0, 0.5, 1.0, 0.8}),
        EffectParameter::makeBool("wireframe", "Wireframe", true),
        EffectParameter::makeAngle("rotation_y", "Rotation Y", 0.0)
    }}};
}

std::unique_ptr<Effect> MirEffect::clone() const {
    auto e = std::make_unique<MirEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void MirEffect::render(PixelBuffer& buffer, double time) {
    if (buffer.width == 0 || buffer.height == 0) return;
    int vertX = getIntParam("vertices_x");
    int vertY = getIntParam("vertices_y");
    double h = getFloatParam("height");
    double freq = getFloatParam("frequency");
    double spd = getFloatParam("speed");
    Color mc = getColorParam("color");
    bool wire = getBoolParam("wireframe");
    double rotY = getAngleParam("rotation_y") * M_PI / 180.0;
    double cr = mc.r * 255.0;
    double cg = mc.g * 255.0;
    double cb = mc.b * 255.0;
    double cx = buffer.width / 2.0;
    struct Vert { double sx, sy; double height; };
    std::vector<std::vector<Vert>> verts(vertY, std::vector<Vert>(vertX));
    for (int vy = 0; vy < vertY; vy++) {
        for (int vx = 0; vx < vertX; vx++) {
            double u = static_cast<double>(vx) / (vertX - 1);
            double v = static_cast<double>(vy) / (vertY - 1);
            double worldX = (u - 0.5) * buffer.width;
            double worldZ = (v - 0.5) * buffer.width;
            double rotatedX = worldX * std::cos(rotY) - worldZ * std::sin(rotY);
            double rotatedZ = worldX * std::sin(rotY) + worldZ * std::cos(rotY);
            double perspScale = 1.0 / (1.0 + rotatedZ * 0.001);
            double waveH = h * std::sin(u * freq * 6.28318 + time * spd) *
                           std::cos(v * freq * 6.28318 + time * spd * 0.7);
            verts[vy][vx].sx = cx + rotatedX * perspScale;
            verts[vy][vx].sy = buffer.height * 0.7 - waveH * perspScale;
            verts[vy][vx].height = std::abs(waveH) / h;
        }
    }
    auto drawLine = [&](int x0, int y0, int x1, int y1, double alpha) {
        int steps = std::max(std::abs(x1 - x0), std::abs(y1 - y0)) + 1;
        for (int s = 0; s < steps; s++) {
            double t = steps > 1 ? static_cast<double>(s) / (steps - 1) : 0.0;
            int px = static_cast<int>(x0 + (x1 - x0) * t);
            int py = static_cast<int>(y0 + (y1 - y0) * t);
            if (px < 0 || px >= buffer.width || py < 0 || py >= buffer.height) continue;
            uint8_t* p = buffer.pixelAt(px, py);
            double sa = p[3] / 255.0;
            double outA = sa + alpha * (1.0 - sa);
            if (outA > 0.001) {
                p[0] = static_cast<uint8_t>((p[0] * sa + cr * alpha * (1.0 - sa)) / outA);
                p[1] = static_cast<uint8_t>((p[1] * sa + cg * alpha * (1.0 - sa)) / outA);
                p[2] = static_cast<uint8_t>((p[2] * sa + cb * alpha * (1.0 - sa)) / outA);
            }
            p[3] = static_cast<uint8_t>(std::clamp(outA * 255.0, 0.0, 255.0));
        }
    };
    for (int vy = 0; vy < vertY; vy++) {
        for (int vx = 0; vx < vertX; vx++) {
            double alpha = mc.a * (0.3 + verts[vy][vx].height * 0.7);
            if (vx < vertX - 1) {
                drawLine(static_cast<int>(verts[vy][vx].sx), static_cast<int>(verts[vy][vx].sy),
                         static_cast<int>(verts[vy][vx + 1].sx), static_cast<int>(verts[vy][vx + 1].sy), alpha);
            }
            if (vy < vertY - 1) {
                drawLine(static_cast<int>(verts[vy][vx].sx), static_cast<int>(verts[vy][vx].sy),
                         static_cast<int>(verts[vy + 1][vx].sx), static_cast<int>(verts[vy + 1][vx].sy), alpha);
            }
            if (!wire) {
                int ipx = static_cast<int>(verts[vy][vx].sx);
                int ipy = static_cast<int>(verts[vy][vx].sy);
                int ptRad = 2;
                for (int dy = -ptRad; dy <= ptRad; dy++) {
                    for (int dx = -ptRad; dx <= ptRad; dx++) {
                        int sx = ipx + dx;
                        int sy = ipy + dy;
                        if (sx < 0 || sx >= buffer.width || sy < 0 || sy >= buffer.height) continue;
                        double dist = std::sqrt(dx * dx + dy * dy);
                        if (dist > ptRad) continue;
                        double falloff = 1.0 - dist / (ptRad + 0.5);
                        double pa = alpha * falloff;
                        uint8_t* p = buffer.pixelAt(sx, sy);
                        double sa = p[3] / 255.0;
                        double outA = sa + pa * (1.0 - sa);
                        if (outA > 0.001) {
                            p[0] = static_cast<uint8_t>((p[0] * sa + cr * pa * (1.0 - sa)) / outA);
                            p[1] = static_cast<uint8_t>((p[1] * sa + cg * pa * (1.0 - sa)) / outA);
                            p[2] = static_cast<uint8_t>((p[2] * sa + cb * pa * (1.0 - sa)) / outA);
                        }
                        p[3] = static_cast<uint8_t>(std::clamp(outA * 255.0, 0.0, 255.0));
                    }
                }
            }
        }
    }
}

} // namespace FreeEffect
