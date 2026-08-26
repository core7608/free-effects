#include "../effect_registry.h"
#include "mosaic_effect.h"
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<MosaicEffect> s_reg("Mosaic", "Stylize");

MosaicEffect::MosaicEffect() {
    addParameter(EffectParameter::makeInt("horizontalBlocks", "Horizontal Blocks", 1, 4096, 40));
    addParameter(EffectParameter::makeInt("verticalBlocks", "Vertical Blocks", 1, 4096, 30));
    addParameter(EffectParameter::makeBool("sharpColors", "Sharp Colors", true));
}

std::vector<ParameterGroup> MosaicEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeInt("horizontalBlocks", "Horizontal Blocks", 1, 4096, false),
        EffectParameter::makeInt("verticalBlocks", "Vertical Blocks", 1, 4096, false),
        EffectParameter::makeBool("sharpColors", "Sharp Colors", true)
    }}};
}

std::unique_ptr<Effect> MosaicEffect::clone() const {
    auto e = std::make_unique<MosaicEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void MosaicEffect::render(PixelBuffer& buffer, double time) {
    int hBlocks = std::max(getIntParam("horizontalBlocks"), 1);
    int vBlocks = std::max(getIntParam("verticalBlocks"), 1);
    bool sharp = getBoolParam("sharpColors");

    int cellW = buffer.width / hBlocks;
    int cellH = buffer.height / vBlocks;
    if (cellW < 1) cellW = 1;
    if (cellH < 1) cellH = 1;

    for (int by = 0; by < vBlocks; by++) {
        for (int bx = 0; bx < hBlocks; bx++) {
            float r = 0, g = 0, b = 0;
            int count = 0;
            int startX = bx * cellW;
            int startY = by * cellH;
            int endX = std::min(startX + cellW, buffer.width);
            int endY = std::min(startY + cellH, buffer.height);

            for (int y = startY; y < endY; y++) {
                for (int x = startX; x < endX; x++) {
                    const uint8_t* p = buffer.pixelAt(x, y);
                    r += p[0]; g += p[1]; b += p[2];
                    count++;
                }
            }
            if (count > 0) {
                r /= count; g /= count; b /= count;
            }
            uint8_t cr = static_cast<uint8_t>(r);
            uint8_t cg = static_cast<uint8_t>(g);
            uint8_t cb = static_cast<uint8_t>(b);

            for (int y = startY; y < endY; y++) {
                for (int x = startX; x < endX; x++) {
                    uint8_t* p = buffer.pixelAt(x, y);
                    p[0] = cr; p[1] = cg; p[2] = cb;
                }
            }
        }
    }
}

} // namespace FreeEffect
