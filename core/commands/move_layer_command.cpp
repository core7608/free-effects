#include "move_layer_command.h"

namespace FreeEffect {

MoveLayerCommand::MoveLayerCommand(Composition* comp, int fromIndex, int toIndex)
    : m_composition(comp)
    , m_fromIndex(fromIndex)
    , m_toIndex(toIndex) {
}

void MoveLayerCommand::execute() {
    m_composition->moveLayer(m_fromIndex, m_toIndex);
}

void MoveLayerCommand::undo() {
    m_composition->moveLayer(m_toIndex, m_fromIndex);
}

std::string MoveLayerCommand::getDescription() const {
    return "Move Layer";
}

} // namespace FreeEffect
