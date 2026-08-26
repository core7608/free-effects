#include "../effect_registry.h"
#include "offset_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<OffsetEffect> s_reg("Offset", "Distort");

OffsetEffect::OffsetEffect() {
    addParameter(EffectParameter::makeVec2("center", "Center", {0.5, 0.5}));
    addParameter(EffectParameter::makeVec2("shift", "Shift", {0.0, 0.0}));
    addParameter(EffectParameter::makeDropdown("edgeBehavior", "Edge Behavior", {"Wrap Around", "Repeat Edge Pixels"}, 0));
}

std::unique_ptr<Effect> OffsetEffect::clone() const {
    auto e = std::make_unique<OffsetEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void OffsetEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    Vec2 shift = getVec2Param("shift");
    bool wrap = (getDropdownParam("edgeBehavior") == 0);
    int sx = static_cast<int>(shift.x * buffer.width + 0.5f);
    int sy = static_cast<int>(shift.y * buffer.height + 0.5f);

    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);
    std::copy(buffer.data.begin(), buffer.data.end(), tmp.data.begin());

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            int srcX, srcY;
            if (wrap) {
                srcX = ((x + sx) % buffer.width + buffer.width) % buffer.width;
                srcY = ((y + sy) % buffer.height + buffer.height) % buffer.height;
            } else {
                srcX = std::clamp(x + sx, 0, buffer.width - 1);
                srcY = std::clamp(y + sy, 0, buffer.height - 1);
            }
            const uint8_t* p = tmp.pixelAt(srcX, srcY);
            uint8_t* dst = buffer.pixelAt(x, y);
            dst[0] = p[0]; dst[1] = p[1]; dst[2] = p[2]; dst[3] = p[3];
        }
    }
}

} // namespace FreeEffect
