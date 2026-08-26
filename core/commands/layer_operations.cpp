#include "layer_operations.h"
#include <sstream>
#include <algorithm>

namespace FreeEffect {

// ── FlattenLayersCommand ──────────────────────────────────────

FlattenLayersCommand::FlattenLayersCommand(Composition* comp) : m_comp(comp) {}

void FlattenLayersCommand::execute() {
    m_savedLayers.clear();
    const auto& layers = m_comp->getLayers();
    for (const auto& layer : layers) {
        if (layer->isVisible()) {
            m_savedLayers.push_back(layer);
        }
    }

    for (const auto& layer : m_savedLayers) {
        m_comp->removeLayer(layer->getId());
    }

    m_flattenedLayer = m_comp->addLayer("Flattened", LayerType::Precomp);
    m_flattenedLayer->setVisible(true);
    m_executed = true;
}

void FlattenLayersCommand::undo() {
    if (!m_executed) return;
    if (m_flattenedLayer) {
        m_comp->removeLayer(m_flattenedLayer->getId());
        m_flattenedLayer = nullptr;
    }
    for (auto it = m_savedLayers.rbegin(); it != m_savedLayers.rend(); ++it) {
        m_comp->addLayer((*it)->getName(), (*it)->getType());
    }
    m_executed = false;
}

std::string FlattenLayersCommand::getDescription() const { return "Flatten Layers"; }

// ── CreateNullObjectCommand ───────────────────────────────────

CreateNullObjectCommand::CreateNullObjectCommand(Composition* comp, double time)
    : m_comp(comp), m_time(time) {}

void CreateNullObjectCommand::execute() {
    m_layer = m_comp->addLayer("Null " + std::to_string(m_comp->getLayerCount() + 1), LayerType::Null);
    m_layer->setStartTime(m_time);
    m_layer->setVisible(true);
}

void CreateNullObjectCommand::undo() {
    if (m_layer) { m_comp->removeLayer(m_layer->getId()); m_layer = nullptr; }
}

std::string CreateNullObjectCommand::getDescription() const { return "Create Null Object"; }

// ── CreateSolidLayerCommand ───────────────────────────────────

CreateSolidLayerCommand::CreateSolidLayerCommand(Composition* comp, const std::string& name,
                                                  const Color& color, int width, int height)
    : m_comp(comp), m_name(name), m_color(color), m_width(width), m_height(height) {}

void CreateSolidLayerCommand::execute() {
    m_layer = m_comp->addLayer(m_name, LayerType::Solid);
    m_layer->setVisible(true);
}

void CreateSolidLayerCommand::undo() {
    if (m_layer) { m_comp->removeLayer(m_layer->getId()); m_layer = nullptr; }
}

std::string CreateSolidLayerCommand::getDescription() const { return "Create Solid: " + m_name; }

// ── CreateAdjustmentLayerCommand ──────────────────────────────

CreateAdjustmentLayerCommand::CreateAdjustmentLayerCommand(Composition* comp) : m_comp(comp) {}

void CreateAdjustmentLayerCommand::execute() {
    m_layer = m_comp->addLayer("Adjustment Layer " + std::to_string(m_comp->getLayerCount() + 1),
                                LayerType::Adjustment);
    m_layer->setAdjustmentLayer(true);
    m_layer->setVisible(true);
}

void CreateAdjustmentLayerCommand::undo() {
    if (m_layer) { m_comp->removeLayer(m_layer->getId()); m_layer = nullptr; }
}

std::string CreateAdjustmentLayerCommand::getDescription() const { return "Create Adjustment Layer"; }

// ── CreateCameraLayerCommand ──────────────────────────────────

CreateCameraLayerCommand::CreateCameraLayerCommand(Composition* comp, const std::string& name)
    : m_comp(comp), m_name(name) {}

void CreateCameraLayerCommand::execute() {
    m_layer = m_comp->addLayer(m_name, LayerType::Camera);
    m_layer->set3D(true);
    m_layer->setVisible(true);
}

void CreateCameraLayerCommand::undo() {
    if (m_layer) { m_comp->removeLayer(m_layer->getId()); m_layer = nullptr; }
}

std::string CreateCameraLayerCommand::getDescription() const { return "Create Camera: " + m_name; }

// ── CreateLightLayerCommand ───────────────────────────────────

CreateLightLayerCommand::CreateLightLayerCommand(Composition* comp, const std::string& name, int lightType)
    : m_comp(comp), m_name(name), m_lightType(lightType) {
    (void)m_lightType;
}

void CreateLightLayerCommand::execute() {
    m_layer = m_comp->addLayer(m_name, LayerType::Light);
    m_layer->set3D(true);
    m_layer->setVisible(true);
}

void CreateLightLayerCommand::undo() {
    if (m_layer) { m_comp->removeLayer(m_layer->getId()); m_layer = nullptr; }
}

std::string CreateLightLayerCommand::getDescription() const { return "Create Light: " + m_name; }

// ── CreateTextLayerCommand ────────────────────────────────────

CreateTextLayerCommand::CreateTextLayerCommand(Composition* comp, const std::string& name)
    : m_comp(comp), m_name(name) {}

void CreateTextLayerCommand::execute() {
    m_layer = m_comp->addLayer(m_name, LayerType::Text);
    m_layer->setVisible(true);
}

void CreateTextLayerCommand::undo() {
    if (m_layer) { m_comp->removeLayer(m_layer->getId()); m_layer = nullptr; }
}

std::string CreateTextLayerCommand::getDescription() const { return "Create Text Layer: " + m_name; }

// ── CreateShapeLayerCommand ───────────────────────────────────

CreateShapeLayerCommand::CreateShapeLayerCommand(Composition* comp, const std::string& name)
    : m_comp(comp), m_name(name) {}

void CreateShapeLayerCommand::execute() {
    m_layer = m_comp->addLayer(m_name, LayerType::Shape);
    m_layer->setVisible(true);
}

void CreateShapeLayerCommand::undo() {
    if (m_layer) { m_comp->removeLayer(m_layer->getId()); m_layer = nullptr; }
}

std::string CreateShapeLayerCommand::getDescription() const { return "Create Shape Layer: " + m_name; }

// ── CreateNullAtPointCommand ──────────────────────────────────

CreateNullAtPointCommand::CreateNullAtPointCommand(Composition* comp, double x, double y, double time)
    : m_comp(comp), m_x(x), m_y(y), m_time(time) {}

void CreateNullAtPointCommand::execute() {
    m_layer = m_comp->addLayer("Null " + std::to_string(m_comp->getLayerCount() + 1), LayerType::Null);
    m_layer->setStartTime(m_time);
    m_layer->getPosition().setDefaultValue(m_x);
    m_layer->setVisible(true);
}

void CreateNullAtPointCommand::undo() {
    if (m_layer) { m_comp->removeLayer(m_layer->getId()); m_layer = nullptr; }
}

std::string CreateNullAtPointCommand::getDescription() const { return "Create Null at Point"; }

// ── DuplicateLayerCommand ─────────────────────────────────────

DuplicateLayerCommand::DuplicateLayerCommand(Composition* comp, int layerIndex)
    : m_comp(comp), m_sourceIndex(layerIndex), m_insertIndex(layerIndex + 1) {}

void DuplicateLayerCommand::execute() {
    auto src = m_comp->getLayer(m_sourceIndex);
    if (!src) return;

    std::string dupName = src->getName() + " Duplicate";
    int suffix = 2;
    while (m_comp->getLayerByName(dupName)) {
        dupName = src->getName() + " Duplicate " + std::to_string(suffix++);
    }

    m_newLayer = m_comp->addLayer(dupName, src->getType());
    m_newLayer->setStartTime(src->getStartTime());
    m_newLayer->setDuration(src->getDuration());
    m_newLayer->setSourcePath(src->getSourcePath());
    m_newLayer->setVisible(src->isVisible());
    m_newLayer->setAudioEnabled(src->isAudioEnabled());
    m_newLayer->setBlendMode(src->getBlendMode());
    m_newLayer->setLocked(false);
    m_newLayer->setSolo(src->isSolo());
    m_newLayer->set3D(src->is3D());
    m_newLayer->setAdjustmentLayer(src->isAdjustmentLayer());
    m_newLayer->setMotionBlurEnabled(src->isMotionBlurEnabled());
    m_newLayer->setShy(src->isShy());

    m_newLayer->getPosition() = src->getPosition();
    m_newLayer->getScale() = src->getScale();
    m_newLayer->getRotation() = src->getRotation();
    m_newLayer->getOpacity() = src->getOpacity();
    m_newLayer->getAnchorPoint() = src->getAnchorPoint();

    int currentIdx = m_comp->getLayerCount() - 1;
    if (m_insertIndex <= currentIdx && m_insertIndex != currentIdx) {
        m_comp->moveLayer(currentIdx, m_insertIndex);
    }
}

void DuplicateLayerCommand::undo() {
    if (m_newLayer) { m_comp->removeLayer(m_newLayer->getId()); m_newLayer = nullptr; }
}

std::string DuplicateLayerCommand::getDescription() const { return "Duplicate Layer"; }

// ── RenameLayerCommand ────────────────────────────────────────

RenameLayerCommand::RenameLayerCommand(Composition* comp, const UUID& layerId, const std::string& newName)
    : m_comp(comp), m_layerId(layerId), m_newName(newName) {}

void RenameLayerCommand::execute() {
    auto layer = m_comp->getLayerById(m_layerId);
    if (!layer) return;
    m_oldName = layer->getName();
    layer->setName(m_newName);
}

void RenameLayerCommand::undo() {
    auto layer = m_comp->getLayerById(m_layerId);
    if (layer) layer->setName(m_oldName);
}

std::string RenameLayerCommand::getDescription() const { return "Rename Layer"; }

// ── SetInOutPointsCommand ─────────────────────────────────────

SetInOutPointsCommand::SetInOutPointsCommand(Composition* comp, int layerIndex,
                                              double inTime, double outTime)
    : m_comp(comp), m_layerIndex(layerIndex), m_inTime(inTime), m_outTime(outTime) {}

void SetInOutPointsCommand::execute() {
    auto layer = m_comp->getLayer(m_layerIndex);
    if (!layer) return;
    m_oldStartTime = layer->getStartTime();
    m_oldDuration = layer->getDuration();
    layer->setStartTime(m_inTime);
    layer->setDuration(m_outTime - m_inTime);
}

void SetInOutPointsCommand::undo() {
    auto layer = m_comp->getLayer(m_layerIndex);
    if (!layer) return;
    layer->setStartTime(m_oldStartTime);
    layer->setDuration(m_oldDuration);
}

std::string SetInOutPointsCommand::getDescription() const { return "Set In/Out Points"; }

// ── TrimLayerToCurrentTimeCommand ─────────────────────────────

TrimLayerToCurrentTimeCommand::TrimLayerToCurrentTimeCommand(Composition* comp, int layerIndex,
                                                              double currentTime, bool trimEnd)
    : m_comp(comp), m_layerIndex(layerIndex), m_currentTime(currentTime), m_trimEnd(trimEnd) {}

void TrimLayerToCurrentTimeCommand::execute() {
    auto layer = m_comp->getLayer(m_layerIndex);
    if (!layer) return;
    m_oldStartTime = layer->getStartTime();
    m_oldDuration = layer->getDuration();

    if (m_trimEnd) {
        double newDuration = m_currentTime - layer->getStartTime();
        if (newDuration > 0) layer->setDuration(newDuration);
    } else {
        double newStart = m_currentTime;
        double endTime = m_oldStartTime + m_oldDuration;
        layer->setStartTime(newStart);
        layer->setDuration(endTime - newStart);
    }
}

void TrimLayerToCurrentTimeCommand::undo() {
    auto layer = m_comp->getLayer(m_layerIndex);
    if (!layer) return;
    layer->setStartTime(m_oldStartTime);
    layer->setDuration(m_oldDuration);
}

std::string TrimLayerToCurrentTimeCommand::getDescription() const { return "Trim Layer"; }

// ── SlideLayerCommand ─────────────────────────────────────────

SlideLayerCommand::SlideLayerCommand(Composition* comp, int layerIndex, double deltaTime)
    : m_comp(comp), m_layerIndex(layerIndex), m_deltaTime(deltaTime) {}

void SlideLayerCommand::execute() {
    auto layer = m_comp->getLayer(m_layerIndex);
    if (layer) layer->setStartTime(layer->getStartTime() + m_deltaTime);
}

void SlideLayerCommand::undo() {
    auto layer = m_comp->getLayer(m_layerIndex);
    if (layer) layer->setStartTime(layer->getStartTime() - m_deltaTime);
}

std::string SlideLayerCommand::getDescription() const {
    return m_deltaTime > 0 ? "Slide Layer Forward" : "Slide Layer Backward";
}

// ── RippleDeleteCommand ───────────────────────────────────────

RippleDeleteCommand::RippleDeleteCommand(Composition* comp, int layerIndex)
    : m_comp(comp), m_layerIndex(layerIndex) {}

void RippleDeleteCommand::execute() {
    auto layer = m_comp->getLayer(m_layerIndex);
    if (!layer) return;

    m_removedLayer = layer;
    m_removedIndex = m_layerIndex;
    m_removedStartTime = layer->getStartTime();
    m_removedDuration = layer->getDuration();

    m_comp->removeLayer(layer->getId());

    m_shiftedLayers.clear();
    auto& layers = m_comp->getLayers();
    for (int i = 0; i < static_cast<int>(layers.size()); ++i) {
        if (layers[i]->getStartTime() > m_removedStartTime) {
            m_shiftedLayers.emplace_back(i, layers[i]->getStartTime());
            layers[i]->setStartTime(layers[i]->getStartTime() - m_removedDuration);
        }
    }
}

void RippleDeleteCommand::undo() {
    for (auto it = m_shiftedLayers.rbegin(); it != m_shiftedLayers.rend(); ++it) {
        auto layer = m_comp->getLayer(it->first);
        if (layer) layer->setStartTime(it->second);
    }
    m_shiftedLayers.clear();

    if (m_removedLayer) {
        m_comp->addLayer(m_removedLayer->getName(), m_removedLayer->getType());
        int idx = m_comp->getLayerCount() - 1;
        auto restored = m_comp->getLayer(idx);
        if (restored) {
            restored->setStartTime(m_removedStartTime);
            restored->setDuration(m_removedDuration);
            restored->setVisible(m_removedLayer->isVisible());
            restored->setBlendMode(m_removedLayer->getBlendMode());
        }
        if (m_removedIndex < idx) {
            m_comp->moveLayer(idx, m_removedIndex);
        }
        m_removedLayer = nullptr;
    }
}

std::string RippleDeleteCommand::getDescription() const { return "Ripple Delete Layer"; }

// ── EnableTimeRemapCommand ────────────────────────────────────

EnableTimeRemapCommand::EnableTimeRemapCommand(Composition* comp, int layerIndex, bool enable)
    : m_comp(comp), m_layerIndex(layerIndex), m_enable(enable) {}

void EnableTimeRemapCommand::execute() {
    auto layer = m_comp->getLayer(m_layerIndex);
    if (!layer) return;
    m_wasEnabled = layer->isTimeRemapEnabled();
    layer->setTimeRemapEnabled(m_enable);
}

void EnableTimeRemapCommand::undo() {
    auto layer = m_comp->getLayer(m_layerIndex);
    if (layer) layer->setTimeRemapEnabled(m_wasEnabled);
}

std::string EnableTimeRemapCommand::getDescription() const {
    return m_enable ? "Enable Time Remap" : "Disable Time Remap";
}

// ── AddLayerMarkerCommand ─────────────────────────────────────

AddLayerMarkerCommand::AddLayerMarkerCommand(Composition* comp, int layerIndex, double time,
                                              const std::string& name, const std::string& comment)
    : m_comp(comp), m_layerIndex(layerIndex), m_time(time), m_name(name), m_comment(comment) {}

void AddLayerMarkerCommand::execute() {
    auto layer = m_comp->getLayer(m_layerIndex);
    if (!layer) return;
    layer->addMarker(m_time, m_name, m_comment);
    m_markerIndex = static_cast<int>(layer->getMarkers().size()) - 1;
}

void AddLayerMarkerCommand::undo() {
    auto layer = m_comp->getLayer(m_layerIndex);
    if (layer && m_markerIndex >= 0) {
        layer->removeMarker(m_markerIndex);
    }
}

std::string AddLayerMarkerCommand::getDescription() const { return "Add Layer Marker"; }

// ── ConvertToSoloCommand ──────────────────────────────────────

ConvertToSoloCommand::ConvertToSoloCommand(Composition* comp, int layerIndex, bool solo)
    : m_comp(comp), m_layerIndex(layerIndex), m_solo(solo) {}

void ConvertToSoloCommand::execute() {
    m_prevSoloStates.clear();
    auto& layers = m_comp->getLayers();

    if (m_solo) {
        for (int i = 0; i < static_cast<int>(layers.size()); ++i) {
            m_prevSoloStates.emplace_back(i, layers[i]->isSolo());
        }
        for (auto& layer : layers) {
            layer->setSolo(false);
        }
    }

    auto target = m_comp->getLayer(m_layerIndex);
    if (target) target->setSolo(m_solo);
}

void ConvertToSoloCommand::undo() {
    auto& layers = m_comp->getLayers();
    for (const auto& [idx, wasSolo] : m_prevSoloStates) {
        if (idx < static_cast<int>(layers.size())) {
            layers[idx]->setSolo(wasSolo);
        }
    }
}

std::string ConvertToSoloCommand::getDescription() const {
    return m_solo ? "Solo Layer" : "Unsolo Layer";
}

// ── HideOtherLayersCommand ────────────────────────────────────

HideOtherLayersCommand::HideOtherLayersCommand(Composition* comp, int layerIndex)
    : m_comp(comp), m_layerIndex(layerIndex) {}

void HideOtherLayersCommand::execute() {
    m_prevVisibility.clear();
    auto& layers = m_comp->getLayers();
    for (int i = 0; i < static_cast<int>(layers.size()); ++i) {
        if (i != m_layerIndex) {
            m_prevVisibility.emplace_back(i, layers[i]->isVisible());
            layers[i]->setVisible(false);
        }
    }
}

void HideOtherLayersCommand::undo() {
    auto& layers = m_comp->getLayers();
    for (const auto& [idx, wasVisible] : m_prevVisibility) {
        if (idx < static_cast<int>(layers.size())) {
            layers[idx]->setVisible(wasVisible);
        }
    }
}

std::string HideOtherLayersCommand::getDescription() const { return "Hide Other Layers"; }

// ── LockOtherLayersCommand ────────────────────────────────────

LockOtherLayersCommand::LockOtherLayersCommand(Composition* comp, int layerIndex)
    : m_comp(comp), m_layerIndex(layerIndex) {}

void LockOtherLayersCommand::execute() {
    m_prevLockStates.clear();
    auto& layers = m_comp->getLayers();
    for (int i = 0; i < static_cast<int>(layers.size()); ++i) {
        if (i != m_layerIndex) {
            m_prevLockStates.emplace_back(i, layers[i]->isLocked());
            layers[i]->setLocked(true);
        }
    }
}

void LockOtherLayersCommand::undo() {
    auto& layers = m_comp->getLayers();
    for (const auto& [idx, wasLocked] : m_prevLockStates) {
        if (idx < static_cast<int>(layers.size())) {
            layers[idx]->setLocked(wasLocked);
        }
    }
}

std::string LockOtherLayersCommand::getDescription() const { return "Lock Other Layers"; }

// ── ShyLayerCommand ───────────────────────────────────────────

ShyLayerCommand::ShyLayerCommand(Composition* comp, int layerIndex)
    : m_comp(comp), m_layerIndex(layerIndex) {}

void ShyLayerCommand::execute() {
    auto layer = m_comp->getLayer(m_layerIndex);
    if (!layer) return;
    m_prevShy = layer->isShy();
    layer->setShy(!m_prevShy);
}

void ShyLayerCommand::undo() {
    auto layer = m_comp->getLayer(m_layerIndex);
    if (layer) layer->setShy(m_prevShy);
}

std::string ShyLayerCommand::getDescription() const { return "Toggle Shy Layer"; }

// ── SetQualityCommand ─────────────────────────────────────────

SetQualityCommand::SetQualityCommand(Composition* comp, int layerIndex, int quality)
    : m_comp(comp), m_layerIndex(layerIndex), m_quality(quality) {}

void SetQualityCommand::execute() {
    auto layer = m_comp->getLayer(m_layerIndex);
    if (!layer) return;
    m_prevQuality = layer->getQuality();
    layer->setQuality(m_quality);
}

void SetQualityCommand::undo() {
    auto layer = m_comp->getLayer(m_layerIndex);
    if (layer) layer->setQuality(m_prevQuality);
}

std::string SetQualityCommand::getDescription() const {
    switch (m_quality) {
        case 0: return "Set Quality: Best";
        case 1: return "Set Quality: Draft";
        case 2: return "Set Quality: Wireframe";
        default: return "Set Quality";
    }
}

// ── SetBlendingModeCommand ────────────────────────────────────

SetBlendingModeCommand::SetBlendingModeCommand(Composition* comp, int layerIndex, BlendMode mode)
    : m_comp(comp), m_layerIndex(layerIndex), m_mode(mode) {}

void SetBlendingModeCommand::execute() {
    auto layer = m_comp->getLayer(m_layerIndex);
    if (!layer) return;
    m_prevMode = layer->getBlendMode();
    layer->setBlendMode(m_mode);
}

void SetBlendingModeCommand::undo() {
    auto layer = m_comp->getLayer(m_layerIndex);
    if (layer) layer->setBlendMode(m_prevMode);
}

std::string SetBlendingModeCommand::getDescription() const { return "Set Blending Mode"; }

// ── SetTrackMatteCommand ──────────────────────────────────────

SetTrackMatteCommand::SetTrackMatteCommand(Composition* comp, int layerIndex,
                                            int matteIndex, TrackMatteMode mode)
    : m_comp(comp), m_layerIndex(layerIndex), m_matteIndex(matteIndex), m_mode(mode) {}

void SetTrackMatteCommand::execute() {
    auto layer = m_comp->getLayer(m_layerIndex);
    if (!layer) return;
    (void)m_prevMatte;
    // Track matte state is stored per-layer; store previous and apply new
}

void SetTrackMatteCommand::undo() {
    // Restore previous matte state
}

std::string SetTrackMatteCommand::getDescription() const {
    switch (m_mode) {
        case TrackMatteMode::AlphaMatte: return "Set Track Matte: Alpha";
        case TrackMatteMode::AlphaInvertedMatte: return "Set Track Matte: Alpha Inverted";
        case TrackMatteMode::LumaMatte: return "Set Track Matte: Luma";
        case TrackMatteMode::LumaInvertedMatte: return "Set Track Matte: Luma Inverted";
        default: return "Set Track Matte: None";
    }
}

} // namespace FreeEffect
