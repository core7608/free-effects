#include "property_track.h"
#include <algorithm>
#include <stdexcept>

namespace FreeEffect {

PropertyTrack::PropertyTrack(const std::string& name)
    : m_name(name) {
}

void PropertyTrack::addKeyframe(const Keyframe& keyframe) {
    auto it = findIteratorAtTime(keyframe.getTime());
    if (it != m_keyframes.end() && std::abs(it->getTime() - keyframe.getTime()) < 1e-6) {
        *it = keyframe;
    } else {
        m_keyframes.insert(it, keyframe);
    }
}

void PropertyTrack::removeKeyframe(double time) {
    auto it = findIteratorAtTime(time);
    if (it != m_keyframes.end() && std::abs(it->getTime() - time) < 1e-6) {
        m_keyframes.erase(it);
    }
}

std::optional<Keyframe> PropertyTrack::findKeyframe(double time) const {
    auto it = findIteratorAtTime(time);
    if (it != m_keyframes.end() && std::abs(it->getTime() - time) < 1e-6) {
        return *it;
    }
    return std::nullopt;
}

double PropertyTrack::getValueAtTime(double time) const {
    if (m_keyframes.empty()) {
        return m_defaultValue;
    }
    
    if (time <= m_keyframes.front().getTime()) {
        return m_keyframes.front().getValue();
    }
    
    if (time >= m_keyframes.back().getTime()) {
        return m_keyframes.back().getValue();
    }
    
    auto it = findIteratorAtTime(time);
    if (it == m_keyframes.end()) {
        return m_defaultValue;
    }
    
    if (std::abs(it->getTime() - time) < 1e-6) {
        return it->getValue();
    }
    
    auto prev = std::prev(it);
    return prev->interpolate(*it, time);
}

KeyframeList::iterator PropertyTrack::findIteratorAtTime(double time) {
    return std::lower_bound(m_keyframes.begin(), m_keyframes.end(), time,
        [](const Keyframe& kf, double t) { return kf.getTime() < t; });
}

KeyframeList::const_iterator PropertyTrack::findIteratorAtTime(double time) const {
    return std::lower_bound(m_keyframes.begin(), m_keyframes.end(), time,
        [](const Keyframe& kf, double t) { return kf.getTime() < t; });
}

} // namespace FreeEffect
