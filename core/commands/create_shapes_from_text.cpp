#include "create_shapes_from_text.h"
#include <cmath>

namespace FreeEffect {

CreateShapesFromTextCommand::CreateShapesFromTextCommand(
    Composition* comp, int layerIndex)
    : m_comp(comp)
    , m_layerIndex(layerIndex) {
}

void CreateShapesFromTextCommand::execute() {
    const auto& layers = m_comp->getLayers();
    if (m_layerIndex < 0 || m_layerIndex >= static_cast<int>(layers.size())) return;

    auto layer = layers[m_layerIndex];
    if (layer->getType() != LayerType::Text) return;

    m_originalTextLayer = layer;
    layer->setVisible(false);

    m_shapeLayer = m_comp->addLayer(layer->getName() + " [shapes]", LayerType::Shape);
    m_shapeLayer->setStartTime(layer->getStartTime());
    m_shapeLayer->setDuration(layer->getDuration());
    m_shapeLayer->getPosition() = layer->getPosition();
    m_shapeLayer->getScale() = layer->getScale();
    m_shapeLayer->getRotation() = layer->getRotation();
    m_shapeLayer->getOpacity() = layer->getOpacity();
    m_shapeLayer->getAnchorPoint() = layer->getAnchorPoint();

    auto textShapes = generateTextShapes();

    m_executed = true;
}

void CreateShapesFromTextCommand::undo() {
    if (m_executed) {
        if (m_shapeLayer) {
            m_comp->removeLayer(m_shapeLayer->getId());
            m_shapeLayer = nullptr;
        }
        if (m_originalTextLayer) {
            m_originalTextLayer->setVisible(true);
        }
    }
    m_executed = false;
}

std::string CreateShapesFromTextCommand::getDescription() const {
    return "Create Shapes from Text";
}

std::vector<ShapeGroup> CreateShapesFromTextCommand::generateTextShapes() const {
    std::vector<ShapeGroup> shapes;

    if (!m_originalTextLayer) return shapes;

    std::string text = "Text";

    double charWidth = 40.0;
    double charHeight = 50.0;
    double spacing = 45.0;
    double startX = 0.0;
    double y = 0.0;

    for (size_t i = 0; i < text.size(); i++) {
        double x = startX + i * spacing;
        shapes.push_back(createCharacterShape(text[i], x, y, charWidth, charHeight));
    }

    return shapes;
}

ShapeGroup CreateShapesFromTextCommand::createCharacterShape(
    char ch, double x, double y, double w, double h) const {

    ShapeGroup group;
    group.name = std::string(1, ch);
    group.fillEnabled = true;
    group.fillColor = {1.0, 1.0, 1.0, 1.0};
    group.strokeEnabled = false;

    ShapePath path;
    path.closed = true;

    double cx = x + w * 0.5;
    double cy = y + h * 0.5;

    path.points = {
        {x, y},
        {x + w, y},
        {x + w, y + h},
        {x, y + h}
    };

    group.paths.push_back(path);
    return group;
}

} // namespace FreeEffect
