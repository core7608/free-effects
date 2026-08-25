#include "remove_layer_command.h"

namespace FreeEffect {

RemoveLayerCommand::RemoveLayerCommand(Composition* comp, const UUID& layerId)
    : m_composition(comp)
    , m_layerId(layerId) {
}

void RemoveLayerCommand::execute() {
    m_removedLayer = m_composition->getLayerById(m_layerId);
    if (m_removedLayer) {
        const auto& layers = m_composition->getLayers();
        for (int i = 0; i < static_cast<int>(layers.size()); ++i) {
            if (layers[i]->getId() == m_layerId) {
                m_layerIndex = i;
                break;
            }
        }
        m_composition->removeLayer(m_layerId);
    }
}

void RemoveLayerCommand::undo() {
    if (m_removedLayer) {
        restoreLayer();
    }
}

void RemoveLayerCommand::restoreLayer() {
    // Temporarily store the layer reference
    LayerPtr restoredLayer = m_removedLayer;
    
    // Add it back at the end (addLayer puts it at the end)
    auto added = m_composition->addLayer(restoredLayer->getName(), restoredLayer->getType());
    
    // Move to original position if needed
    int currentIdx = m_composition->getLayerCount() - 1;
    if (m_layerIndex >= 0 && m_layerIndex < currentIdx) {
        m_composition->moveLayer(currentIdx, m_layerIndex);
    }
    
    // Restore all properties from the saved layer
    auto& target = *m_composition->getLayer(m_layerIndex >= 0 ? m_layerIndex : 0);
    target.setStartTime(restoredLayer->getStartTime());
    target.setDuration(restoredLayer->getDuration());
    target.setSourcePath(restoredLayer->getSourcePath());
    target.setVisible(restoredLayer->isVisible());
    target.setAudioEnabled(restoredLayer->isAudioEnabled());
    target.setBlendMode(restoredLayer->getBlendMode());
    target.setLocked(restoredLayer->isLocked());
    target.setSolo(restoredLayer->isSolo());
    target.setParentLayerId(restoredLayer->getParentLayerId());
    
    // Restore keyframes for all transform properties
    auto copyKeyframes = [](const PropertyTrack& src, PropertyTrack& dst) {
        dst = src;
    };
    
    copyKeyframes(restoredLayer->getPosition(), target.getPosition());
    copyKeyframes(restoredLayer->getScale(), target.getScale());
    copyKeyframes(restoredLayer->getRotation(), target.getRotation());
    copyKeyframes(restoredLayer->getOpacity(), target.getOpacity());
    copyKeyframes(restoredLayer->getAnchorPoint(), target.getAnchorPoint());
}

std::string RemoveLayerCommand::getDescription() const {
    return "Remove Layer";
}

} // namespace FreeEffect
