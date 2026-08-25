#pragma once

#include <QMainWindow>
#include <QDockWidget>
#include <QLabel>
#include <QSplitter>
#include <QToolBar>
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

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;
    
    ProjectState& getProjectState() { return *m_project; }
    CommandStack& getCommandStack() { return *m_commandStack; }
    CompositionPanel* getCompositionPanel() const { return m_compositionPanel; }
    TimelinePanel* getTimelinePanel() const { return m_timelinePanel; }
    ProjectPanel* getProjectPanel() const { return m_projectPanel; }
    EffectControlsPanel* getEffectControlsPanel() const { return m_effectControlsPanel; }
    
    void updateTitle();
    
    void onNewComposition();
    void onImportFile();
    void onAddToRenderQueue();

private:
    void setupUi();
    void setupMenuBar();
    void setupToolBar();
    void setupDockWidgets();
    void setupStatusBar();
    void connectSignals();
    void setupKeyboardShortcuts();
    
    std::unique_ptr<ProjectState> m_project;
    std::unique_ptr<CommandStack> m_commandStack;
    
    MenuBar* m_menuBar;
    QToolBar* m_toolBar;
    
    ProjectPanel* m_projectPanel = nullptr;
    CompositionPanel* m_compositionPanel = nullptr;
    TimelinePanel* m_timelinePanel = nullptr;
    EffectControlsPanel* m_effectControlsPanel = nullptr;
    
    QDockWidget* m_projectDock = nullptr;
    QDockWidget* m_compositionDock = nullptr;
    QDockWidget* m_timelineDock = nullptr;
    QDockWidget* m_effectControlsDock = nullptr;
};

} // namespace FreeEffect
