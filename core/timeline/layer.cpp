#include "layer.h"

namespace FreeEffect {

Layer::Layer(const std::string& name, LayerType type)
    : m_id(generateUUID())
    , m_name(name)
    , m_type(type)
    , m_position("Position")
    , m_scale("Scale")
    , m_rotation("Rotation")
    , m_opacity("Opacity")
    , m_anchorPoint("Anchor Point") {
    m_scale.setDefaultValue(100.0);
    m_opacity.setDefaultValue(100.0);
}

bool Layer::isActiveAtTime(double time) const {
    return time >= m_startTime && time <= m_startTime + m_duration;
}

} // namespace FreeEffect
