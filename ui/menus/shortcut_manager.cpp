#include "shortcut_manager.h"

namespace FreeEffect {

ShortcutManager& ShortcutManager::instance() {
    static ShortcutManager mgr;
    return mgr;
}

ShortcutManager::ShortcutManager() {
    registerDefaults();
}

void ShortcutManager::registerDefaults() {
    // File
    m_shortcuts[static_cast<int>(ShortcutAction::NewProject)] = QKeySequence("Ctrl+Alt+N");
    m_shortcuts[static_cast<int>(ShortcutAction::OpenProject)] = QKeySequence("Ctrl+O");
    m_shortcuts[static_cast<int>(ShortcutAction::Save)] = QKeySequence("Ctrl+S");
    m_shortcuts[static_cast<int>(ShortcutAction::SaveAs)] = QKeySequence("Ctrl+Shift+S");
    m_shortcuts[static_cast<int>(ShortcutAction::ImportFile)] = QKeySequence("Ctrl+I");
    m_shortcuts[static_cast<int>(ShortcutAction::AddToRenderQueue)] = QKeySequence("Ctrl+M");
    
    // Edit
    m_shortcuts[static_cast<int>(ShortcutAction::Undo)] = QKeySequence("Ctrl+Z");
    m_shortcuts[static_cast<int>(ShortcutAction::Redo)] = QKeySequence("Ctrl+Shift+Z");
    m_shortcuts[static_cast<int>(ShortcutAction::Cut)] = QKeySequence("Ctrl+X");
    m_shortcuts[static_cast<int>(ShortcutAction::Copy)] = QKeySequence("Ctrl+C");
    m_shortcuts[static_cast<int>(ShortcutAction::Paste)] = QKeySequence("Ctrl+V");
    m_shortcuts[static_cast<int>(ShortcutAction::Duplicate)] = QKeySequence("Ctrl+D");
    m_shortcuts[static_cast<int>(ShortcutAction::SelectAll)] = QKeySequence("Ctrl+A");
    m_shortcuts[static_cast<int>(ShortcutAction::DeselectAll)] = QKeySequence("Ctrl+Shift+A");
    
    // Composition
    m_shortcuts[static_cast<int>(ShortcutAction::NewComposition)] = QKeySequence("Ctrl+N");
    m_shortcuts[static_cast<int>(ShortcutAction::CompositionSettings)] = QKeySequence("Ctrl+K");
    
    // Layer properties (press single key)
    m_shortcuts[static_cast<int>(ShortcutAction::ShowPosition)] = QKeySequence("P");
    m_shortcuts[static_cast<int>(ShortcutAction::ShowScale)] = QKeySequence("S");
    m_shortcuts[static_cast<int>(ShortcutAction::ShowRotation)] = QKeySequence("R");
    m_shortcuts[static_cast<int>(ShortcutAction::ShowOpacity)] = QKeySequence("T");
    m_shortcuts[static_cast<int>(ShortcutAction::ShowAnchorPoint)] = QKeySequence("A");
    
    // Tools
    m_shortcuts[static_cast<int>(ShortcutAction::ToolSelection)] = QKeySequence("V");
    m_shortcuts[static_cast<int>(ShortcutAction::ToolHand)] = QKeySequence("H");
    m_shortcuts[static_cast<int>(ShortcutAction::ToolZoom)] = QKeySequence("Z");
    m_shortcuts[static_cast<int>(ShortcutAction::ToolRotation)] = QKeySequence("W");
    m_shortcuts[static_cast<int>(ShortcutAction::ToolPen)] = QKeySequence("G");
    m_shortcuts[static_cast<int>(ShortcutAction::ToolText)] = QKeySequence("Ctrl+T");
    
    // Timeline
    m_shortcuts[static_cast<int>(ShortcutAction::PlayPause)] = QKeySequence("Space");
    m_shortcuts[static_cast<int>(ShortcutAction::GoToNextKeyframe)] = QKeySequence("K");
    m_shortcuts[static_cast<int>(ShortcutAction::GoToPrevKeyframe)] = QKeySequence("J");
    m_shortcuts[static_cast<int>(ShortcutAction::GoToStart)] = QKeySequence("Home");
    m_shortcuts[static_cast<int>(ShortcutAction::GoToEnd)] = QKeySequence("End");
    m_shortcuts[static_cast<int>(ShortcutAction::TrimWorkAreaStart)] = QKeySequence("B");
    m_shortcuts[static_cast<int>(ShortcutAction::TrimWorkAreaEnd)] = QKeySequence("N");
    m_shortcuts[static_cast<int>(ShortcutAction::ZoomTimelineIn)] = QKeySequence("+");
    m_shortcuts[static_cast<int>(ShortcutAction::ZoomTimelineOut)] = QKeySequence("-");
    m_shortcuts[static_cast<int>(ShortcutAction::RevealAllAnimated)] = QKeySequence("U");
    
    // View
    m_shortcuts[static_cast<int>(ShortcutAction::ZoomInView)] = QKeySequence("=");
    m_shortcuts[static_cast<int>(ShortcutAction::ZoomOutView)] = QKeySequence("-");
    m_shortcuts[static_cast<int>(ShortcutAction::FitToWindow)] = QKeySequence("Ctrl+Shift+/");
    m_shortcuts[static_cast<int>(ShortcutAction::ToggleGrid)] = QKeySequence("Ctrl+'");
    m_shortcuts[static_cast<int>(ShortcutAction::ToggleRulers)] = QKeySequence("Ctrl+R");
    
    // Layer
    m_shortcuts[static_cast<int>(ShortcutAction::LockLayer)] = QKeySequence("Ctrl+L");
}

QKeySequence ShortcutManager::getShortcut(ShortcutAction action) const {
    auto it = m_shortcuts.find(static_cast<int>(action));
    return (it != m_shortcuts.end()) ? it->second : QKeySequence();
}

void ShortcutManager::setShortcut(ShortcutAction action, const QKeySequence& sequence) {
    m_shortcuts[static_cast<int>(action)] = sequence;
}

QString ShortcutManager::getActionName(ShortcutAction action) {
    switch (action) {
        case ShortcutAction::NewProject: return "New Project";
        case ShortcutAction::OpenProject: return "Open Project";
        case ShortcutAction::Save: return "Save";
        case ShortcutAction::SaveAs: return "Save As";
        case ShortcutAction::ImportFile: return "Import File";
        case ShortcutAction::AddToRenderQueue: return "Add to Render Queue";
        case ShortcutAction::Undo: return "Undo";
        case ShortcutAction::Redo: return "Redo";
        case ShortcutAction::Cut: return "Cut";
        case ShortcutAction::Copy: return "Copy";
        case ShortcutAction::Paste: return "Paste";
        case ShortcutAction::Duplicate: return "Duplicate";
        case ShortcutAction::SelectAll: return "Select All";
        case ShortcutAction::DeselectAll: return "Deselect All";
        case ShortcutAction::NewComposition: return "New Composition";
        case ShortcutAction::CompositionSettings: return "Composition Settings";
        case ShortcutAction::ShowPosition: return "Show Position";
        case ShortcutAction::ShowScale: return "Show Scale";
        case ShortcutAction::ShowRotation: return "Show Rotation";
        case ShortcutAction::ShowOpacity: return "Show Opacity";
        case ShortcutAction::ShowAnchorPoint: return "Show Anchor Point";
        case ShortcutAction::ToolSelection: return "Selection Tool";
        case ShortcutAction::ToolHand: return "Hand Tool";
        case ShortcutAction::ToolZoom: return "Zoom Tool";
        case ShortcutAction::ToolRotation: return "Rotation Tool";
        case ShortcutAction::ToolPen: return "Pen Tool";
        case ShortcutAction::ToolText: return "Text Tool";
        case ShortcutAction::PlayPause: return "Play/Pause";
        case ShortcutAction::GoToNextKeyframe: return "Next Keyframe";
        case ShortcutAction::GoToPrevKeyframe: return "Previous Keyframe";
        case ShortcutAction::GoToStart: return "Go to Start";
        case ShortcutAction::GoToEnd: return "Go to End";
        case ShortcutAction::TrimWorkAreaStart: return "Trim Work Area Start";
        case ShortcutAction::TrimWorkAreaEnd: return "Trim Work Area End";
        case ShortcutAction::ZoomTimelineIn: return "Zoom Timeline In";
        case ShortcutAction::ZoomTimelineOut: return "Zoom Timeline Out";
        case ShortcutAction::RevealAllAnimated: return "Reveal All Animated";
        case ShortcutAction::ZoomInView: return "Zoom In";
        case ShortcutAction::ZoomOutView: return "Zoom Out";
        case ShortcutAction::FitToWindow: return "Fit to Window";
        case ShortcutAction::ToggleGrid: return "Toggle Grid";
        case ShortcutAction::ToggleRulers: return "Toggle Rulers";
        case ShortcutAction::LockLayer: return "Lock Layer";
    }
    return "Unknown";
}

QString ShortcutManager::getShortcutString(ShortcutAction action) {
    return instance().getShortcut(action).toString(QKeySequence::NativeText);
}

} // namespace FreeEffect
