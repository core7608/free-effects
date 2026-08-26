#include "layer.h"
#include "../effects/effect.h"
#include <algorithm>
#include <stdexcept>

namespace FreeEffect {

Layer::Layer(const std::string& name, LayerType type)
    : m_id(generateUUID())
    , m_name(name)
    , m_type(type)
    , m_position("Position")
    , m_scale("Scale")
    , m_rotation("Rotation")
    , m_opacity("Opacity")
    , m_anchorPoint("Anchor Point")
    , m_timeRemap("Time Remap") {
    m_scale.setDefaultValue(100.0);
    m_opacity.setDefaultValue(100.0);
}

bool Layer::isActiveAtTime(double time) const {
    return time >= m_startTime && time <= m_startTime + m_duration;
}

void Layer::addEffect(std::shared_ptr<Effect> effect) {
    if (!effect) return;
    effect->setOrder(static_cast<int>(m_effects.size()));
    m_effects.push_back(std::move(effect));
}

void Layer::removeEffect(const UUID& effectId) {
    m_effects.erase(
        std::remove_if(m_effects.begin(), m_effects.end(),
            [&effectId](const std::shared_ptr<Effect>& e) {
                return e->getId() == effectId;
            }),
        m_effects.end());
}

void Layer::removeEffect(int index) {
    if (index >= 0 && index < static_cast<int>(m_effects.size())) {
        m_effects.erase(m_effects.begin() + index);
    }
}

std::shared_ptr<Effect> Layer::getEffect(int index) const {
    if (index >= 0 && index < static_cast<int>(m_effects.size())) {
        return m_effects[index];
    }
    return nullptr;
}

std::shared_ptr<Effect> Layer::getEffectById(const UUID& effectId) const {
    for (const auto& e : m_effects) {
        if (e->getId() == effectId) return e;
    }
    return nullptr;
}

void Layer::moveEffect(int fromIndex, int toIndex) {
    if (fromIndex < 0 || fromIndex >= static_cast<int>(m_effects.size()) ||
        toIndex < 0 || toIndex >= static_cast<int>(m_effects.size()) ||
        fromIndex == toIndex) {
        return;
    }
    auto effect = m_effects[fromIndex];
    m_effects.erase(m_effects.begin() + fromIndex);
    m_effects.insert(m_effects.begin() + toIndex, std::move(effect));
    for (int i = 0; i < static_cast<int>(m_effects.size()); ++i) {
        m_effects[i]->setOrder(i);
    }
}

void Layer::setParentLayer(const std::shared_ptr<Layer>& parent) {
    m_parentLayer = parent;
}

std::shared_ptr<Layer> Layer::getParentLayer() const {
    return m_parentLayer.lock();
}

double Layer::remapTime(double time) const {
    if (!m_timeRemapEnabled) return time;
    double remapValue = m_timeRemap.getValueAtTime(time);
    double layerEnd = m_startTime + m_duration;
    return m_startTime + remapValue * m_duration;
}

void Layer::addMarker(double time, const std::string& name, const std::string& comment) {
    LayerMarker marker;
    marker.time = time;
    marker.name = name;
    marker.comment = comment;
    m_markers.push_back(std::move(marker));
}

void Layer::removeMarker(int index) {
    if (index >= 0 && index < static_cast<int>(m_markers.size())) {
        m_markers.erase(m_markers.begin() + index);
    }
}

} // namespace FreeEffect
