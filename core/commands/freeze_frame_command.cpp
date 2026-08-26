#include "freeze_frame_command.h"

namespace FreeEffect {

FreezeFrameCommand::FreezeFrameCommand(Composition* comp, int layerIndex, double freezeTime)
    : m_comp(comp)
    , m_layerIndex(layerIndex)
    , m_freezeTime(freezeTime) {
}

void FreezeFrameCommand::execute() {
    const auto& layers = m_comp->getLayers();
    if (m_layerIndex < 0 || m_layerIndex >= static_cast<int>(layers.size())) return;

    auto layer = layers[m_layerIndex];
    if (!layer->isActiveAtTime(m_freezeTime)) return;

    m_originalDuration = layer->getDuration();
    m_originalStartTime = layer->getStartTime();
    m_originalTimeRemapEnabled = layer->isTimeRemapEnabled();

    double relativeTime = m_freezeTime - layer->getStartTime();

    layer->setTimeRemapEnabled(true);
    layer->getTimeRemap().setDefaultValue(relativeTime);

    double remainingDuration = m_originalDuration - relativeTime;
    if (remainingDuration > 0) {
        layer->setDuration(remainingDuration);
        layer->setStartTime(m_freezeTime);
    }

    m_executed = true;
}

void FreezeFrameCommand::undo() {
    if (!m_executed) return;

    const auto& layers = m_comp->getLayers();
    if (m_layerIndex >= 0 && m_layerIndex < static_cast<int>(layers.size())) {
        auto layer = layers[m_layerIndex];
        layer->setStartTime(m_originalStartTime);
        layer->setDuration(m_originalDuration);
        layer->setTimeRemapEnabled(m_originalTimeRemapEnabled);
    }
    m_executed = false;
}

std::string FreezeFrameCommand::getDescription() const {
    return "Freeze Frame";
}

} // namespace FreeEffect
