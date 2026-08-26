#pragma once

#include "property_track.h"
#include "transform_3d.h"
#include "types.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace FreeEffect {

class Effect;

struct LayerMarker {
    double time = 0.0;
    std::string name;
    std::string comment;
    bool protectedRegion = false;
};

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
    
    void setParentLayer(const std::shared_ptr<Layer>& parent);
    std::shared_ptr<Layer> getParentLayer() const;

    void setParentIndex(int idx) { m_parentIndex = idx; }
    int getParentIndex() const { return m_parentIndex; }

    const UUID& getId() const { return m_id; }

    void set3D(bool is3D) { m_is3D = is3D; m_transform3D.is3D = is3D; }
    bool is3D() const { return m_is3D; }

    Transform3D& getTransform3D() { return m_transform3D; }
    const Transform3D& getTransform3D() const { return m_transform3D; }

    void addEffect(std::shared_ptr<Effect> effect);
    void removeEffect(const UUID& effectId);
    void removeEffect(int index);
    std::shared_ptr<Effect> getEffect(int index) const;
    std::shared_ptr<Effect> getEffectById(const UUID& effectId) const;
    std::vector<std::shared_ptr<Effect>>& getEffects() { return m_effects; }
    const std::vector<std::shared_ptr<Effect>>& getEffects() const { return m_effects; }
    int getEffectCount() const { return static_cast<int>(m_effects.size()); }
    bool hasEffects() const { return !m_effects.empty(); }
    void moveEffect(int fromIndex, int toIndex);
    void clearEffects() { m_effects.clear(); }

    void setTimeRemapEnabled(bool enabled) { m_timeRemapEnabled = enabled; }
    bool isTimeRemapEnabled() const { return m_timeRemapEnabled; }
    PropertyTrack& getTimeRemap() { return m_timeRemap; }
    const PropertyTrack& getTimeRemap() const { return m_timeRemap; }
    double remapTime(double time) const;

    void setPrecompId(const std::string& id) { m_precompId = id; }
    const std::string& getPrecompId() const { return m_precompId; }

    void addMarker(double time, const std::string& name, const std::string& comment = "");
    void removeMarker(int index);
    const std::vector<LayerMarker>& getMarkers() const { return m_markers; }
    std::vector<LayerMarker>& getMarkers() { return m_markers; }

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
    int m_parentIndex = -1;
    
    PropertyTrack m_position;
    PropertyTrack m_scale;
    PropertyTrack m_rotation;
    PropertyTrack m_opacity;
    PropertyTrack m_anchorPoint;
    
    std::vector<std::shared_ptr<Effect>> m_effects;

    bool m_is3D = false;
    Transform3D m_transform3D;
    std::weak_ptr<Layer> m_parentLayer;

    bool m_timeRemapEnabled = false;
    PropertyTrack m_timeRemap;

    std::string m_precompId;

    std::vector<LayerMarker> m_markers;
};

using LayerPtr = std::shared_ptr<Layer>;

} // namespace FreeEffect
