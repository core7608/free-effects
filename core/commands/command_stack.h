#pragma once

#include "command.h"
#include <vector>
#include <string>
#include <functional>

namespace FreeEffect {

class CommandStack {
public:
    CommandStack(size_t maxUndoLevels = 32);
    
    void execute(CommandPtr command);
    void undo();
    void redo();
    
    bool canUndo() const;
    bool canRedo() const;
    
    std::string getUndoDescription() const;
    std::string getRedoDescription() const;
    
    void clear();
    
    void setMaxUndoLevels(size_t levels) { m_maxUndoLevels = levels; }
    size_t getMaxUndoLevels() const { return m_maxUndoLevels; }
    
    // Callbacks for UI updates
    void setStateChangedCallback(std::function<void()> callback) { m_stateChangedCallback = callback; }

private:
    void notifyStateChanged();
    
    std::vector<CommandPtr> m_undoStack;
    std::vector<CommandPtr> m_redoStack;
    size_t m_maxUndoLevels;
    std::function<void()> m_stateChangedCallback;
};

} // namespace FreeEffect
