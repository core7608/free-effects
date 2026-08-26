#pragma once

#include <QMainWindow>
#include <QDockWidget>
#include <QLabel>
#include <QSplitter>
#include <QToolBar>
#include <QActionGroup>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <memory>
#include "../../core/project/project_state.h"
#include "../../core/commands/command_stack.h"
#include "../menus/menu_bar.h"

namespace FreeEffect {

class ProjectPanel;
class CompositionPanel;
class TimelinePanel;
class EffectControlsPanel;
class EffectsBrowserPanel;
class CanvasWidget;
class AIPanel;
class AICommandExecutor;
class InfoPanel;
class AudioPanel;
class PreviewPanel;
class CharacterPanel;
class ParagraphPanel;
class BrushesPanel;
class PaintPanel;
class SmootherPanel;
class MotionSketchPanel;
class WigglerPanel;
class PosterizeTimePanel;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;
    
    ProjectState& getProjectState() { return *m_project; }
    ProjectState* getProjectStatePtr() { return m_project.get(); }
    CommandStack& getCommandStack() { return *m_commandStack; }
    CompositionPanel* getCompositionPanel() const { return m_compositionPanel; }
    TimelinePanel* getTimelinePanel() const { return m_timelinePanel; }
    ProjectPanel* getProjectPanel() const { return m_projectPanel; }
    EffectControlsPanel* getEffectControlsPanel() const { return m_effectControlsPanel; }
    EffectsBrowserPanel* getEffectsBrowserPanel() const { return m_effectsBrowserPanel; }
    AICommandExecutor* getAICommandExecutor() const;
    InfoPanel* getInfoPanel() const { return m_infoPanel; }
    AudioPanel* getAudioPanel() const { return m_audioPanel; }
    PreviewPanel* getPreviewPanel() const { return m_previewPanel; }
    
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
    
    // Canvas access
    CanvasWidget* getCanvasWidget() const;

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
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    
    std::unique_ptr<ProjectState> m_project;
    std::unique_ptr<CommandStack> m_commandStack;
    
    MenuBar* m_menuBar = nullptr;
    QToolBar* m_toolBar = nullptr;
    QActionGroup* m_toolGroup = nullptr;
    
    ProjectPanel* m_projectPanel = nullptr;
    CompositionPanel* m_compositionPanel = nullptr;
    TimelinePanel* m_timelinePanel = nullptr;
    EffectControlsPanel* m_effectControlsPanel = nullptr;
    EffectsBrowserPanel* m_effectsBrowserPanel = nullptr;
    AIPanel* m_aiPanel = nullptr;
    
    InfoPanel* m_infoPanel = nullptr;
    AudioPanel* m_audioPanel = nullptr;
    PreviewPanel* m_previewPanel = nullptr;
    CharacterPanel* m_characterPanel = nullptr;
    ParagraphPanel* m_paragraphPanel = nullptr;
    BrushesPanel* m_brushesPanel = nullptr;
    PaintPanel* m_paintPanel = nullptr;
    SmootherPanel* m_smootherPanel = nullptr;
    MotionSketchPanel* m_motionSketchPanel = nullptr;
    WigglerPanel* m_wigglerPanel = nullptr;
    PosterizeTimePanel* m_posterizeTimePanel = nullptr;
    
    QDockWidget* m_projectDock = nullptr;
    QDockWidget* m_compositionDock = nullptr;
    QDockWidget* m_timelineDock = nullptr;
    QDockWidget* m_effectControlsDock = nullptr;
    QDockWidget* m_effectsBrowserDock = nullptr;
    QDockWidget* m_aiDock = nullptr;
    
    QDockWidget* m_infoDock = nullptr;
    QDockWidget* m_audioDock = nullptr;
    QDockWidget* m_previewDock = nullptr;
    QDockWidget* m_characterDock = nullptr;
    QDockWidget* m_paragraphDock = nullptr;
    QDockWidget* m_brushesDock = nullptr;
    QDockWidget* m_paintDock = nullptr;
    QDockWidget* m_smootherDock = nullptr;
    QDockWidget* m_motionSketchDock = nullptr;
    QDockWidget* m_wigglerDock = nullptr;
    QDockWidget* m_posterizeTimeDock = nullptr;
    
    int m_selectedLayerIndex = -1;
};

} // namespace FreeEffect
