#include "split_layer_command.h"

namespace FreeEffect {

SplitLayerCommand::SplitLayerCommand(Composition* comp, int layerIndex, double time)
    : m_comp(comp)
    , m_layerIndex(layerIndex)
    , m_splitTime(time) {
}

void SplitLayerCommand::execute() {
    const auto& layers = m_comp->getLayers();
    if (m_layerIndex < 0 || m_layerIndex >= static_cast<int>(layers.size())) return;

    auto layer = layers[m_layerIndex];
    if (m_splitTime <= layer->getStartTime() ||
        m_splitTime >= layer->getStartTime() + layer->getDuration()) return;

    m_originalName = layer->getName();
    m_originalStartTime = layer->getStartTime();
    m_originalDuration = layer->getDuration();
    m_layerType = layer->getType();

    double firstDuration = m_splitTime - layer->getStartTime();
    double secondDuration = layer->getDuration() - firstDuration;

    layer->setDuration(firstDuration);

    m_newLayer = m_comp->addLayer(m_originalName + " [split]", m_layerType);
    m_newLayer->setStartTime(m_splitTime);
    m_newLayer->setDuration(secondDuration);
    m_newLayer->setVisible(layer->isVisible());
    m_newLayer->setSourcePath(layer->getSourcePath());
    m_newLayer->getPosition() = layer->getPosition();
    m_newLayer->getScale() = layer->getScale();
    m_newLayer->getRotation() = layer->getRotation();
    m_newLayer->getOpacity() = layer->getOpacity();
    m_newLayer->getAnchorPoint() = layer->getAnchorPoint();

    int currentIdx = m_comp->getLayerCount() - 1;
    if (m_layerIndex < currentIdx) {
        m_comp->moveLayer(currentIdx, m_layerIndex + 1);
    }
}

void SplitLayerCommand::undo() {
    if (m_newLayer) {
        m_comp->removeLayer(m_newLayer->getId());
        m_newLayer = nullptr;
    }

    const auto& layers = m_comp->getLayers();
    if (m_layerIndex >= 0 && m_layerIndex < static_cast<int>(layers.size())) {
        layers[m_layerIndex]->setDuration(m_originalDuration);
    }
}

std::string SplitLayerCommand::getDescription() const {
    return "Split Layer";
}

} // namespace FreeEffect
