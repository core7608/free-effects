#include "remove_keyframe_command.h"

namespace FreeEffect {

RemoveKeyframeCommand::RemoveKeyframeCommand(Layer* layer, const std::string& propertyName, double time)
    : m_layer(layer)
    , m_propertyName(propertyName)
    , m_time(time) {
}

PropertyTrack* RemoveKeyframeCommand::getPropertyTrack() const {
    if (m_propertyName == "Position") return &m_layer->getPosition();
    if (m_propertyName == "Scale") return &m_layer->getScale();
    if (m_propertyName == "Rotation") return &m_layer->getRotation();
    if (m_propertyName == "Opacity") return &m_layer->getOpacity();
    if (m_propertyName == "Anchor Point") return &m_layer->getAnchorPoint();
    return nullptr;
}

void RemoveKeyframeCommand::execute() {
    PropertyTrack* track = getPropertyTrack();
    if (!track) return;
    
    // Save the keyframe for undo
    m_removedKeyframe = track->findKeyframe(m_time);
    
    if (m_removedKeyframe.has_value()) {
        track->removeKeyframe(m_time);
    }
}

void RemoveKeyframeCommand::undo() {
    PropertyTrack* track = getPropertyTrack();
    if (!track) return;
    
    if (m_removedKeyframe.has_value()) {
        track->addKeyframe(m_removedKeyframe.value());
    }
}

std::string RemoveKeyframeCommand::getDescription() const {
    return "Remove Keyframe: " + m_propertyName;
}

} // namespace FreeEffect
