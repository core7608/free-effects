#include "command_stack.h"
#include <stdexcept>

namespace FreeEffect {

CommandStack::CommandStack(size_t maxUndoLevels)
    : m_maxUndoLevels(maxUndoLevels) {
}

void CommandStack::execute(CommandPtr command) {
    if (!command) {
        return;
    }
    
    command->execute();
    m_undoStack.push_back(command);
    m_redoStack.clear();
    
    // Trim undo stack if it exceeds max size
    while (m_undoStack.size() > m_maxUndoLevels) {
        m_undoStack.erase(m_undoStack.begin());
    }
    
    notifyStateChanged();
}

void CommandStack::undo() {
    if (!canUndo()) {
        throw std::runtime_error("Nothing to undo");
    }
    
    auto command = m_undoStack.back();
    m_undoStack.pop_back();
    command->undo();
    m_redoStack.push_back(command);
    
    notifyStateChanged();
}

void CommandStack::redo() {
    if (!canRedo()) {
        throw std::runtime_error("Nothing to redo");
    }
    
    auto command = m_redoStack.back();
    m_redoStack.pop_back();
    command->execute();
    m_undoStack.push_back(command);
    
    notifyStateChanged();
}

bool CommandStack::canUndo() const {
    return !m_undoStack.empty();
}

bool CommandStack::canRedo() const {
    return !m_redoStack.empty();
}

std::string CommandStack::getUndoDescription() const {
    if (!canUndo()) {
        return "";
    }
    return m_undoStack.back()->getDescription();
}

std::string CommandStack::getRedoDescription() const {
    if (!canRedo()) {
        return "";
    }
    return m_redoStack.back()->getDescription();
}

void CommandStack::clear() {
    m_undoStack.clear();
    m_redoStack.clear();
    notifyStateChanged();
}

void CommandStack::notifyStateChanged() {
    if (m_stateChangedCallback) {
        m_stateChangedCallback();
    }
}

} // namespace FreeEffect
