#include "precompose_command.h"
#include <algorithm>

namespace FreeEffect {

PrecomposeCommand::PrecomposeCommand(
    Composition* comp, const std::vector<int>& layerIndices, const std::string& name)
    : m_comp(comp)
    , m_layerIndices(layerIndices)
    , m_name(name) {
    std::sort(m_layerIndices.begin(), m_layerIndices.end());
}

void PrecomposeCommand::execute() {
    const auto& layers = m_comp->getLayers();

    m_originalLayers.clear();
    m_originalStartTimes.clear();
    m_originalDurations.clear();

    for (int idx : m_layerIndices) {
        if (idx >= 0 && idx < static_cast<int>(layers.size())) {
            m_originalLayers.push_back(layers[idx]);
            m_originalStartTimes.push_back(layers[idx]->getStartTime());
            m_originalDurations.push_back(layers[idx]->getDuration());
        }
    }

    if (m_originalLayers.empty()) return;

    m_insertIndex = m_layerIndices[0];

    m_nestedComp = std::make_shared<Composition>(
        m_name,
        m_comp->getResolution(),
        m_comp->getFrameRate(),
        m_comp->getDuration());

    double earliestStart = m_originalLayers[0]->getStartTime();
    double latestEnd = m_originalLayers[0]->getStartTime() + m_originalLayers[0]->getDuration();
    for (const auto& layer : m_originalLayers) {
        earliestStart = std::min(earliestStart, layer->getStartTime());
        double endTime = layer->getStartTime() + layer->getDuration();
        latestEnd = std::max(latestEnd, endTime);
    }

    for (auto& layer : m_originalLayers) {
        double newStart = layer->getStartTime() - earliestStart;
        auto newLayer = m_nestedComp->addLayer(layer->getName(), layer->getType());
        newLayer->setStartTime(newStart);
        newLayer->setDuration(layer->getDuration());
        newLayer->setVisible(layer->isVisible());
        newLayer->setSourcePath(layer->getSourcePath());
        newLayer->getPosition() = layer->getPosition();
        newLayer->getScale() = layer->getScale();
        newLayer->getRotation() = layer->getRotation();
        newLayer->getOpacity() = layer->getOpacity();
        newLayer->getAnchorPoint() = layer->getAnchorPoint();
    }

    m_nestedComp->setDuration(latestEnd - earliestStart);

    for (const auto& layer : m_originalLayers) {
        m_comp->removeLayer(layer->getId());
    }

    m_precompLayer = m_comp->addLayer(m_name, LayerType::Precomp);
    m_precompLayer->setStartTime(earliestStart);
    m_precompLayer->setDuration(latestEnd - earliestStart);
    m_precompLayer->setPrecompId(m_nestedComp->getId());

    int currentIndex = m_comp->getLayerCount() - 1;
    if (m_insertIndex < currentIndex) {
        m_comp->moveLayer(currentIndex, m_insertIndex);
    }

    m_comp->addPrecomp(m_nestedComp);
}

void PrecomposeCommand::undo() {
    if (m_precompLayer) {
        m_comp->removeLayer(m_precompLayer->getId());
        m_precompLayer = nullptr;
    }

    for (int i = static_cast<int>(m_originalLayers.size()) - 1; i >= 0; i--) {
        auto& layer = m_originalLayers[i];
        auto newLayer = m_comp->addLayer(layer->getName(), layer->getType());
        newLayer->setStartTime(m_originalStartTimes[i]);
        newLayer->setDuration(m_originalDurations[i]);
        newLayer->setVisible(layer->isVisible());
        newLayer->setSourcePath(layer->getSourcePath());
        newLayer->getPosition() = layer->getPosition();
        newLayer->getScale() = layer->getScale();
        newLayer->getRotation() = layer->getRotation();
        newLayer->getOpacity() = layer->getOpacity();
        newLayer->getAnchorPoint() = layer->getAnchorPoint();
    }
}

std::string PrecomposeCommand::getDescription() const {
    return "Precompose: " + m_name;
}

} // namespace FreeEffect
