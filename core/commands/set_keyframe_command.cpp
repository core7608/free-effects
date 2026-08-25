#include "set_keyframe_command.h"

namespace FreeEffect {

SetKeyframeCommand::SetKeyframeCommand(Layer* layer, const std::string& propertyName,
                                       double time, double value, InterpolationType interp)
    : m_layer(layer)
    , m_propertyName(propertyName)
    , m_time(time)
    , m_value(value)
    , m_interpolation(interp) {
}

PropertyTrack* SetKeyframeCommand::getPropertyTrack() const {
    if (m_propertyName == "Position") return &m_layer->getPosition();
    if (m_propertyName == "Scale") return &m_layer->getScale();
    if (m_propertyName == "Rotation") return &m_layer->getRotation();
    if (m_propertyName == "Opacity") return &m_layer->getOpacity();
    if (m_propertyName == "Anchor Point") return &m_layer->getAnchorPoint();
    return nullptr;
}

void SetKeyframeCommand::execute() {
    PropertyTrack* track = getPropertyTrack();
    if (!track) return;
    
    // Save previous state for undo
    m_previousKeyframe = track->findKeyframe(m_time);
    m_hadPreviousKeyframe = m_previousKeyframe.has_value();
    
    // Add or update keyframe
    track->addKeyframe(Keyframe(m_time, m_value, m_interpolation));
}

void SetKeyframeCommand::undo() {
    PropertyTrack* track = getPropertyTrack();
    if (!track) return;
    
    if (m_hadPreviousKeyframe) {
        // Restore previous keyframe
        track->addKeyframe(m_previousKeyframe.value());
    } else {
        // Remove the keyframe we added
        track->removeKeyframe(m_time);
    }
}

std::string SetKeyframeCommand::getDescription() const {
    return "Set Keyframe: " + m_propertyName;
}

} // namespace FreeEffect
