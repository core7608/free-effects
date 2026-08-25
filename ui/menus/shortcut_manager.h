#pragma once

#include <QKeySequence>
#include <QString>
#include <unordered_map>

namespace FreeEffect {

enum class ShortcutAction {
    // File
    NewProject,
    OpenProject,
    Save,
    SaveAs,
    ImportFile,
    AddToRenderQueue,
    
    // Edit
    Undo,
    Redo,
    Cut,
    Copy,
    Paste,
    Duplicate,
    SelectAll,
    DeselectAll,
    
    // Composition
    NewComposition,
    CompositionSettings,
    
    // Layer transform properties
    ShowPosition,
    ShowScale,
    ShowRotation,
    ShowOpacity,
    ShowAnchorPoint,
    
    // Tools
    ToolSelection,
    ToolHand,
    ToolZoom,
    ToolRotation,
    ToolPen,
    ToolText,
    
    // Timeline
    PlayPause,
    GoToNextKeyframe,
    GoToPrevKeyframe,
    GoToStart,
    GoToEnd,
    TrimWorkAreaStart,
    TrimWorkAreaEnd,
    ZoomTimelineIn,
    ZoomTimelineOut,
    RevealAllAnimated,
    
    // View
    ZoomInView,
    ZoomOutView,
    FitToWindow,
    ToggleGrid,
    ToggleRulers,
    
    // Layer
    LockLayer,
};

class ShortcutManager {
public:
    static ShortcutManager& instance();
    
    QKeySequence getShortcut(ShortcutAction action) const;
    void setShortcut(ShortcutAction action, const QKeySequence& sequence);
    
    static QString getActionName(ShortcutAction action);
    static QString getShortcutString(ShortcutAction action);

private:
    ShortcutManager();
    void registerDefaults();
    
    std::unordered_map<int, QKeySequence> m_shortcuts;
};

} // namespace FreeEffect
