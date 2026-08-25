#pragma once

#include "layer.h"
#include "types.h"
#include <memory>
#include <string>
#include <vector>

namespace FreeEffect {

class Composition {
public:
    Composition(const std::string& name, Resolution resolution, FrameRate frameRate, double duration);
    
    const UUID& getId() const { return m_id; }
    
    const std::string& getName() const { return m_name; }
    void setName(const std::string& name) { m_name = name; }
    
    Resolution getResolution() const { return m_resolution; }
    void setResolution(Resolution res) { m_resolution = res; }
    
    FrameRate getFrameRate() const { return m_frameRate; }
    void setFrameRate(FrameRate rate) { m_frameRate = rate; }
    
    double getDuration() const { return m_duration; }
    void setDuration(double duration) { m_duration = duration; }
    
    Color getBackgroundColor() const { return m_backgroundColor; }
    void setBackgroundColor(Color color) { m_backgroundColor = color; }
    
    LayerPtr addLayer(const std::string& name, LayerType type);
    void removeLayer(const UUID& layerId);
    
    LayerPtr getLayerById(const UUID& layerId) const;
    LayerPtr getLayerByName(const std::string& name) const;
    LayerPtr getLayer(int index) const;
    
    int getLayerCount() const { return static_cast<int>(m_layers.size()); }
    const std::vector<LayerPtr>& getLayers() const { return m_layers; }
    
    void moveLayer(int fromIndex, int toIndex);

private:
    UUID m_id;
    std::string m_name;
    Resolution m_resolution;
    FrameRate m_frameRate;
    double m_duration;
    Color m_backgroundColor;
    std::vector<LayerPtr> m_layers;
};

} // namespace FreeEffect
