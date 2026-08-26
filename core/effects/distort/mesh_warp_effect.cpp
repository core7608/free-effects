#include "../effect_registry.h"
#include "mesh_warp_effect.h"
#include <cmath>
#include <algorithm>
#include <vector>

namespace FreeEffect {

static EffectRegistrar<MeshWarpEffect> s_reg("Mesh Warp", "Distort");

MeshWarpEffect::MeshWarpEffect() {
    addParameter(EffectParameter::makeInt("rows", "Rows", 2, 50, 4));
    addParameter(EffectParameter::makeInt("columns", "Columns", 2, 50, 4));
    addParameter(EffectParameter::makeFloat("elasticity", "Elasticity", 0.0, 100.0, 100.0));
}

std::unique_ptr<Effect> MeshWarpEffect::clone() const {
    auto e = std::make_unique<MeshWarpEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void MeshWarpEffect::render(PixelBuffer& buffer, double time) {
    if (buffer.width == 0 || buffer.height == 0) return;

    int rows = std::max(getIntParam("rows"), 2);
    int cols = std::max(getIntParam("columns"), 2);

    std::vector<std::vector<Vec2>> grid(rows, std::vector<Vec2>(cols));
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            double u = static_cast<double>(c) / (cols - 1);
            double v = static_cast<double>(r) / (rows - 1);
            grid[r][c] = {u * buffer.width, v * buffer.height};
        }
    }

    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            double fx = static_cast<double>(x) / buffer.width * (cols - 1);
            double fy = static_cast<double>(y) / buffer.height * (rows - 1);

            int ix = std::clamp(static_cast<int>(fx), 0, cols - 2);
            int iy = std::clamp(static_cast<int>(fy), 0, rows - 2);
            double tx = fx - ix;
            double ty = fy - iy;

            tx = tx * tx * (3.0 - 2.0 * tx);
            ty = ty * ty * (3.0 - 2.0 * ty);

            double sx = (1 - tx) * (1 - ty) * grid[iy][ix].x + tx * (1 - ty) * grid[iy][ix + 1].x +
                        (1 - tx) * ty * grid[iy + 1][ix].x + tx * ty * grid[iy + 1][ix + 1].x;
            double sy = (1 - tx) * (1 - ty) * grid[iy][ix].y + tx * (1 - ty) * grid[iy][ix + 1].y +
                        (1 - tx) * ty * grid[iy + 1][ix].y + tx * ty * grid[iy + 1][ix + 1].y;

            int srcX = std::clamp(static_cast<int>(sx), 0, buffer.width - 1);
            int srcY = std::clamp(static_cast<int>(sy), 0, buffer.height - 1);
            const uint8_t* src = buffer.pixelAt(srcX, srcY);
            uint8_t* dst = tmp.pixelAt(x, y);
            dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3];
        }
    }
    buffer.data = tmp.data;
}

} // namespace FreeEffect
