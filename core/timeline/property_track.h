#pragma once

#include "keyframe.h"
#include <string>
#include <optional>

namespace FreeEffect {

class PropertyTrack {
public:
    explicit PropertyTrack(const std::string& name);
    
    PropertyTrack& operator=(const PropertyTrack& other) {
        if (this != &other) {
            m_keyframes = other.m_keyframes;
            m_defaultValue = other.m_defaultValue;
        }
        return *this;
    }
    
    const std::string& getName() const { return m_name; }
    
    void addKeyframe(const Keyframe& keyframe);
    void removeKeyframe(double time);
    
    std::optional<Keyframe> findKeyframe(double time) const;
    const KeyframeList& getKeyframes() const { return m_keyframes; }
    
    double getValueAtTime(double time) const;
    
    void setDefaultValue(double value) { m_defaultValue = value; }
    double getDefaultValue() const { return m_defaultValue; }
    
    bool hasKeyframes() const { return !m_keyframes.empty(); }

private:
    KeyframeList::iterator findIteratorAtTime(double time);
    KeyframeList::const_iterator findIteratorAtTime(double time) const;
    
    std::string m_name;
    KeyframeList m_keyframes;
    double m_defaultValue = 0.0;
};

} // namespace FreeEffect
