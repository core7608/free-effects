#include "set_property_command.h"

namespace FreeEffect {

SetPropertyCommand::SetPropertyCommand(Layer* layer, const std::string& propertyName, double value)
    : m_layer(layer)
    , m_propertyName(propertyName)
    , m_value(value) {
}

PropertyTrack* SetPropertyCommand::getPropertyTrack() const {
    if (m_propertyName == "Position") return &m_layer->getPosition();
    if (m_propertyName == "Scale") return &m_layer->getScale();
    if (m_propertyName == "Rotation") return &m_layer->getRotation();
    if (m_propertyName == "Opacity") return &m_layer->getOpacity();
    if (m_propertyName == "Anchor Point") return &m_layer->getAnchorPoint();
    return nullptr;
}

void SetPropertyCommand::execute() {
    PropertyTrack* track = getPropertyTrack();
    if (!track) return;
    
    m_previousValue = track->getDefaultValue();
    track->setDefaultValue(m_value);
}

void SetPropertyCommand::undo() {
    PropertyTrack* track = getPropertyTrack();
    if (!track) return;
    
    track->setDefaultValue(m_previousValue);
}

std::string SetPropertyCommand::getDescription() const {
    return "Set Property: " + m_propertyName;
}

} // namespace FreeEffect
