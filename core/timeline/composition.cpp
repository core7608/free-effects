#include "composition.h"
#include <algorithm>
#include <stdexcept>

namespace FreeEffect {

Composition::Composition(const std::string& name, Resolution resolution, FrameRate frameRate, double duration)
    : m_id(generateUUID())
    , m_name(name)
    , m_resolution(resolution)
    , m_frameRate(frameRate)
    , m_duration(duration)
    , m_backgroundColor({0.0, 0.0, 0.0, 1.0}) {
}

LayerPtr Composition::addLayer(const std::string& name, LayerType type) {
    auto layer = std::make_shared<Layer>(name, type);
    m_layers.push_back(layer);
    return layer;
}

void Composition::removeLayer(const UUID& layerId) {
    auto it = std::find_if(m_layers.begin(), m_layers.end(),
        [&layerId](const LayerPtr& layer) { return layer->getId() == layerId; });
    
    if (it != m_layers.end()) {
        m_layers.erase(it);
    }
}

LayerPtr Composition::getLayerById(const UUID& layerId) const {
    auto it = std::find_if(m_layers.begin(), m_layers.end(),
        [&layerId](const LayerPtr& layer) { return layer->getId() == layerId; });
    
    return (it != m_layers.end()) ? *it : nullptr;
}

LayerPtr Composition::getLayerByName(const std::string& name) const {
    auto it = std::find_if(m_layers.begin(), m_layers.end(),
        [&name](const LayerPtr& layer) { return layer->getName() == name; });
    
    return (it != m_layers.end()) ? *it : nullptr;
}

LayerPtr Composition::getLayer(int index) const {
    if (index >= 0 && index < static_cast<int>(m_layers.size())) {
        return m_layers[index];
    }
    return nullptr;
}

void Composition::moveLayer(int fromIndex, int toIndex) {
    if (fromIndex < 0 || fromIndex >= static_cast<int>(m_layers.size()) ||
        toIndex < 0 || toIndex >= static_cast<int>(m_layers.size())) {
        throw std::out_of_range("Layer index out of range");
    }
    
    auto layer = m_layers[fromIndex];
    m_layers.erase(m_layers.begin() + fromIndex);
    m_layers.insert(m_layers.begin() + toIndex, layer);
}

void Composition::addLight(std::shared_ptr<Light> light) {
    if (light) {
        m_lights.push_back(std::move(light));
    }
}

void Composition::removeLight(const UUID& id) {
    m_lights.erase(
        std::remove_if(m_lights.begin(), m_lights.end(),
            [&id](const std::shared_ptr<Light>& l) { return l->getId() == id; }),
        m_lights.end());
}

void Composition::setWorkArea(double start, double end) {
    m_workAreaStart = start;
    m_workAreaEnd = end;
}

double Composition::getWorkAreaDuration() const {
    return m_workAreaEnd - m_workAreaStart;
}

bool Composition::isInWorkArea(double time) const {
    return time >= m_workAreaStart && time <= m_workAreaEnd;
}

void Composition::setParentComposition(const std::shared_ptr<Composition>& parent) {
    m_parentComposition = parent;
}

std::shared_ptr<Composition> Composition::getParentComposition() const {
    return m_parentComposition.lock();
}

void Composition::addPrecomp(std::shared_ptr<Composition> comp) {
    if (comp) {
        m_precomps.push_back(std::move(comp));
    }
}

std::shared_ptr<Composition> Composition::getPrecompById(const std::string& id) const {
    for (const auto& comp : m_precomps) {
        if (comp && comp->getId() == id) {
            return comp;
        }
    }
    return nullptr;
}

} // namespace FreeEffect
