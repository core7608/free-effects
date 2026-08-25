#pragma once

#include "property_track.h"
#include "types.h"
#include <memory>
#include <string>
#include <unordered_map>

namespace FreeEffect {

class Layer {
public:
    Layer(const std::string& name, LayerType type);
    ~Layer() = default;
    
    const std::string& getName() const { return m_name; }
    void setName(const std::string& name) { m_name = name; }
    
    LayerType getType() const { return m_type; }
    
    void setStartTime(double time) { m_startTime = time; }
    double getStartTime() const { return m_startTime; }
    
    void setDuration(double duration) { m_duration = duration; }
    double getDuration() const { return m_duration; }
    
    bool isActiveAtTime(double time) const;
    
    // Transform properties
    PropertyTrack& getPosition() { return m_position; }
    PropertyTrack& getScale() { return m_scale; }
    PropertyTrack& getRotation() { return m_rotation; }
    PropertyTrack& getOpacity() { return m_opacity; }
    PropertyTrack& getAnchorPoint() { return m_anchorPoint; }
    
    const PropertyTrack& getPosition() const { return m_position; }
    const PropertyTrack& getScale() const { return m_scale; }
    const PropertyTrack& getRotation() const { return m_rotation; }
    const PropertyTrack& getOpacity() const { return m_opacity; }
    const PropertyTrack& getAnchorPoint() const { return m_anchorPoint; }
    
    void setSourcePath(const std::string& path) { m_sourcePath = path; }
    const std::string& getSourcePath() const { return m_sourcePath; }
    
    void setVisible(bool visible) { m_visible = visible; }
    bool isVisible() const { return m_visible; }
    
    void setAudioEnabled(bool enabled) { m_audioEnabled = enabled; }
    bool isAudioEnabled() const { return m_audioEnabled; }
    
    void setBlendMode(BlendMode mode) { m_blendMode = mode; }
    BlendMode getBlendMode() const { return m_blendMode; }
    
    void setLocked(bool locked) { m_locked = locked; }
    bool isLocked() const { return m_locked; }
    
    void setSolo(bool solo) { m_solo = solo; }
    bool isSolo() const { return m_solo; }
    
    void setParentLayerId(const UUID& id) { m_parentLayerId = id; }
    const UUID& getParentLayerId() const { return m_parentLayerId; }
    
    const UUID& getId() const { return m_id; }

private:
    UUID m_id;
    std::string m_name;
    LayerType m_type;
    double m_startTime = 0.0;
    double m_duration = 0.0;
    std::string m_sourcePath;
    bool m_visible = true;
    bool m_audioEnabled = true;
    bool m_locked = false;
    bool m_solo = false;
    BlendMode m_blendMode = BlendMode::Normal;
    UUID m_parentLayerId;
    
    // Transform properties
    PropertyTrack m_position;
    PropertyTrack m_scale;
    PropertyTrack m_rotation;
    PropertyTrack m_opacity;
    PropertyTrack m_anchorPoint;
};

using LayerPtr = std::shared_ptr<Layer>;

} // namespace FreeEffect
