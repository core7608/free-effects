#pragma once

#include "camera.h"
#include "layer.h"
#include "light.h"
#include "types.h"
#include <memory>
#include <string>
#include <vector>

namespace FreeEffect {

struct CompMarker {
    double time = 0;
    std::string name;
    std::string comment;
    int index = 0;
    bool protectedRegion = false;
};

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

    void setActiveCamera(std::shared_ptr<Camera> cam) { m_activeCamera = std::move(cam); }
    std::shared_ptr<Camera> getActiveCamera() const { return m_activeCamera; }

    void addLight(std::shared_ptr<Light> light);
    void removeLight(const UUID& id);
    const std::vector<std::shared_ptr<Light>>& getLights() const { return m_lights; }

    void setShutterAngle(double deg) { m_shutterAngle = deg; }
    double getShutterAngle() const { return m_shutterAngle; }

    void setShutterSamples(int s) { m_shutterSamples = s; }
    int getShutterSamples() const { return m_shutterSamples; }

    void setFrameRateValue(double fps) { m_frameRateValue = fps; }
    double getFrameRateValue() const { return m_frameRateValue; }

    void setWorkAreaStart(double t) { m_workAreaStart = t; }
    double getWorkAreaStart() const { return m_workAreaStart; }

    void setWorkAreaEnd(double t) { m_workAreaEnd = t; }
    double getWorkAreaEnd() const { return m_workAreaEnd; }

    void setWorkArea(double start, double end);
    double getWorkAreaDuration() const;
    bool isInWorkArea(double time) const;

    void setParentComposition(const std::shared_ptr<Composition>& parent);
    std::shared_ptr<Composition> getParentComposition() const;

    void addPrecomp(std::shared_ptr<Composition> comp);
    std::shared_ptr<Composition> getPrecompById(const std::string& id) const;
    const std::vector<std::shared_ptr<Composition>>& getPrecomps() const { return m_precomps; }

    // Composition markers
    void addMarker(const CompMarker& marker);
    void removeMarker(int index);
    void removeMarkerByIndex(int markerIndex);
    const std::vector<CompMarker>& getMarkers() const { return m_markers; }
    std::vector<CompMarker>& getMarkers() { return m_markers; }
    CompMarker* getMarkerAtTime(double time, double tolerance = 0.001);
    CompMarker* getMarkerByIndex(int index);

private:
    UUID m_id;
    std::string m_name;
    Resolution m_resolution;
    FrameRate m_frameRate;
    double m_duration;
    Color m_backgroundColor;
    std::vector<LayerPtr> m_layers;

    std::shared_ptr<Camera> m_activeCamera;
    std::vector<std::shared_ptr<Light>> m_lights;

    double m_shutterAngle = 180.0;
    int m_shutterSamples = 16;
    double m_frameRateValue = 30.0;

    double m_workAreaStart = 0.0;
    double m_workAreaEnd = 0.0;

    std::weak_ptr<Composition> m_parentComposition;
    std::vector<std::shared_ptr<Composition>> m_precomps;

    std::vector<CompMarker> m_markers;
};

} // namespace FreeEffect
