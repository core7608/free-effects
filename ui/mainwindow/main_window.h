#pragma once

#include <QMainWindow>
#include <QDockWidget>
#include <QLabel>
#include <QSplitter>
#include <QToolBar>
#include <QActionGroup>
#include <memory>
#include "../../core/project/project_state.h"
#include "../../core/commands/command_stack.h"
#include "../menus/menu_bar.h"

namespace FreeEffect {

class ProjectPanel;
class CompositionPanel;
class TimelinePanel;
class EffectControlsPanel;
class CanvasWidget;
class AIPanel;
class AICommandExecutor;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;
    
    ProjectState& getProjectState() { return *m_project; }
    CommandStack& getCommandStack() { return *m_commandStack; }
    CompositionPanel* getCompositionPanel() const { return m_compositionPanel; }
    TimelinePanel* getTimelinePanel() const { return m_timelinePanel; }
    ProjectPanel* getProjectPanel() const { return m_projectPanel; }
    EffectControlsPanel* getEffectControlsPanel() const { return m_effectControlsPanel; }
    AICommandExecutor* getAICommandExecutor() const;
    
    void updateTitle();
    void refreshAllPanels();
    
    void onNewComposition();
    void onImportFile();
    void onOpenProject(const QString& filePath);
    void onSaveProject();
    void onSaveProjectAs();
    void onAddToRenderQueue();
    void onShowPreferences();
    void onShowAbout();
    void onShowAISettings();
    void onToggleAIPanel();
    void onUndo();
    void onRedo();
    
    void setTool(const QString& toolName);
    void activateToolByName(const QString& name);
    
    void selectLayer(int index);
    void deselectAllLayers();
    
    // Layer operations
    void onNewSolidLayer();
    void onNewTextLayer();
    void onNewNullLayer();
    void onNewAdjustmentLayer();
    void onDeleteSelectedLayer();
    void onDuplicateLayer();
    
    // Edit operations
    void onCut();
    void onCopy();
    void onPaste();
    void onSelectAll();
    void onDeselectAll();
    
    // View operations
    void onZoomIn();
    void onZoomOut();
    void onFitToWindow();
    void onToggleGrid();
    void onToggleRulers();

private:
    void setupUi();
    void setupMenuBar();
    void setupToolBar();
    void setupDockWidgets();
    void setupStatusBar();
    void connectSignals();
    void connectMenuActions();
    void setupKeyboardShortcuts();
    void applyAELayout();
    void addRecentProject(const QString& path);
    
    std::unique_ptr<ProjectState> m_project;
    std::unique_ptr<CommandStack> m_commandStack;
    
    MenuBar* m_menuBar = nullptr;
    QToolBar* m_toolBar = nullptr;
    QActionGroup* m_toolGroup = nullptr;
    
    ProjectPanel* m_projectPanel = nullptr;
    CompositionPanel* m_compositionPanel = nullptr;
    TimelinePanel* m_timelinePanel = nullptr;
    EffectControlsPanel* m_effectControlsPanel = nullptr;
    AIPanel* m_aiPanel = nullptr;
    
    QDockWidget* m_projectDock = nullptr;
    QDockWidget* m_compositionDock = nullptr;
    QDockWidget* m_timelineDock = nullptr;
    QDockWidget* m_effectControlsDock = nullptr;
    QDockWidget* m_aiDock = nullptr;
    
    int m_selectedLayerIndex = -1;
};

} // namespace FreeEffect
