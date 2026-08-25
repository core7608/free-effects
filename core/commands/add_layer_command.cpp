#include "add_layer_command.h"

namespace FreeEffect {

AddLayerCommand::AddLayerCommand(Composition* comp, const std::string& name, LayerType type, int insertIndex)
    : m_composition(comp)
    , m_name(name)
    , m_type(type)
    , m_insertIndex(insertIndex) {
}

void AddLayerCommand::execute() {
    m_layer = m_composition->addLayer(m_name, m_type);
    m_executedIndex = m_composition->getLayerCount() - 1;
    
    if (m_insertIndex >= 0 && m_insertIndex < m_executedIndex) {
        m_composition->moveLayer(m_executedIndex, m_insertIndex);
        m_executedIndex = m_insertIndex;
    }
}

void AddLayerCommand::undo() {
    if (m_layer) {
        m_composition->removeLayer(m_layer->getId());
        m_layer = nullptr;
    }
}

std::string AddLayerCommand::getDescription() const {
    return "Add Layer: " + m_name;
}

} // namespace FreeEffect
