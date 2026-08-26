#include "create_masks_from_text.h"

namespace FreeEffect {

CreateMasksFromTextCommand::CreateMasksFromTextCommand(
    Composition* comp, int layerIndex)
    : m_comp(comp)
    , m_layerIndex(layerIndex) {
}

void CreateMasksFromTextCommand::execute() {
    const auto& layers = m_comp->getLayers();
    if (m_layerIndex < 0 || m_layerIndex >= static_cast<int>(layers.size())) return;

    auto layer = layers[m_layerIndex];
    if (layer->getType() != LayerType::Text) return;

    m_targetLayer = layer;
    m_createdMasks = generateTextMaskPaths();

    m_executed = true;
}

void CreateMasksFromTextCommand::undo() {
    m_createdMasks.clear();
    m_executed = false;
}

std::string CreateMasksFromTextCommand::getDescription() const {
    return "Create Masks from Text";
}

std::vector<ShapePath> CreateMasksFromTextCommand::generateTextMaskPaths() const {
    std::vector<ShapePath> paths;

    if (!m_targetLayer) return paths;

    std::string text = "Text";
    double charWidth = 40.0;
    double charHeight = 50.0;
    double spacing = 45.0;
    double startX = 0.0;
    double y = 0.0;

    for (size_t i = 0; i < text.size(); i++) {
        double x = startX + i * spacing;

        ShapePath path;
        path.closed = true;
        path.points = {
            {x, y},
            {x + charWidth, y},
            {x + charWidth, y + charHeight},
            {x, y + charHeight}
        };

        paths.push_back(path);
    }

    return paths;
}

} // namespace FreeEffect
