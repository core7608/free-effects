#include "../effect_registry.h"
#include "cell_pattern_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<CellPatternEffect> s_reg("Cell Pattern", "Generate");

CellPatternEffect::CellPatternEffect() {
    addParameter(EffectParameter::makeDropdown("pattern", "Pattern", {"Bubbles", "Crystals", "Plates"}, 0));
    addParameter(EffectParameter::makeFloat("size", "Size", 1.0, 500.0, 50.0));
    addParameter(EffectParameter::makeFloat("border", "Border", 0.0, 100.0, 0.5));
    addParameter(EffectParameter::makeColor("color", "Color", {255.0, 255.0, 255.0, 1.0}));
    addParameter(EffectParameter::makeInt("seed", "Random Seed", 0, 9999, 42));
}

std::vector<ParameterGroup> CellPatternEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeDropdown("pattern", "Pattern", {"Bubbles", "Crystals", "Plates"}, 0),
        EffectParameter::makeFloat("size", "Size", 1.0, 500.0, false),
        EffectParameter::makeFloat("border", "Border", 0.0, 100.0, false),
        EffectParameter::makeColor("color", "Color", {255.0, 255.0, 255.0, 1.0}),
        EffectParameter::makeInt("seed", "Random Seed", 0, 9999, false)
    }}};
}

std::unique_ptr<Effect> CellPatternEffect::clone() const {
    auto e = std::make_unique<CellPatternEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void CellPatternEffect::render(PixelBuffer& buffer, double time) {
    float size = getFloatParam("size");
    float border = getFloatParam("border") / 100.0f;
    Color c = getColorParam("color");
    int seed = getIntParam("seed");
    int type = getDropdownParam("pattern");

    int gridSize = static_cast<int>(size) + 1;
    if (gridSize < 2) gridSize = 2;

    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    int gw = (buffer.width / gridSize) + 3;
    int gh = (buffer.height / gridSize) + 3;
    std::vector<float> pointsX(gw * gh);
    std::vector<float> pointsY(gw * gh);
    for (int i = 0; i < gw * gh; i++) {
        pointsX[i] = dist(rng) * gridSize;
        pointsY[i] = dist(rng) * gridSize;
    }

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            float minDist = 1e10f;
            float secondDist = 1e10f;
            int gridX = x / gridSize;
            int gridY = y / gridSize;

            for (int gy = gridY - 1; gy <= gridY + 1; gy++) {
                for (int gx = gridX - 1; gx <= gridX + 1; gx++) {
                    if (gx < 0 || gx >= gw || gy < 0 || gy >= gh) continue;
                    int idx = gy * gw + gx;
                    float px = gx * gridSize + pointsX[idx];
                    float py = gy * gridSize + pointsY[idx];
                    float dx = x - px;
                    float dy = y - py;
                    float d;
                    if (type == 0) d = std::sqrt(dx * dx + dy * dy);
                    else if (type == 1) d = std::abs(dx) + std::abs(dy);
                    else d = std::max(std::abs(dx), std::abs(dy));

                    if (d < minDist) {
                        secondDist = minDist;
                        minDist = d;
                    } else if (d < secondDist) {
                        secondDist = d;
                    }
                }
            }

            float edge = secondDist - minDist;
            float val = 1.0f - std::clamp(edge / (size * 0.2f), 0.0f, 1.0f);
            float cellBright = 1.0f - (minDist / size);
            cellBright = std::clamp(cellBright, 0.0f, 1.0f);

            float alpha = cellBright;
            if (edge < border * size) {
                alpha = 1.0f;
            }

            uint8_t* p = buffer.pixelAt(x, y);
            p[0] = static_cast<uint8_t>(std::clamp(static_cast<double>(c.r * alpha), 0.0, 255.0));
            p[1] = static_cast<uint8_t>(std::clamp(static_cast<double>(c.g * alpha), 0.0, 255.0));
            p[2] = static_cast<uint8_t>(std::clamp(static_cast<double>(c.b * alpha), 0.0, 255.0));
            p[3] = static_cast<uint8_t>(std::clamp(static_cast<double>(alpha * 255.0), 0.0, 255.0));
        }
    }
}

} // namespace FreeEffect
