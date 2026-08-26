#include "main_window.h"
#include "../panels/project_panel.h"
#include "../panels/composition_panel.h"
#include "../panels/timeline_panel.h"
#include "../panels/effect_controls_panel.h"
#include "../panels/effects_browser_panel.h"
#include "../panels/timeline_canvas.h"
#include "../panels/info_panel.h"
#include "../panels/audio_panel.h"
#include "../panels/preview_panel.h"
#include "../panels/character_panel.h"
#include "../panels/paragraph_panel.h"
#include "../panels/brushes_panel.h"
#include "../panels/paint_panel.h"
#include "../panels/smoother_panel.h"
#include "../panels/motion_sketch_panel.h"
#include "../panels/wiggler_panel.h"
#include "../panels/posterize_time_panel.h"
#include "../canvas/canvas_widget.h"
#include "../dialogs/new_composition_dialog.h"
#include "../dialogs/about_dialog.h"
#include "../dialogs/preferences_dialog.h"
#include "../dialogs/render_queue_dialog.h"
#include "../menus/shortcut_manager.h"
#include "../ai/ai_panel.h"
#include "../ai/ai_command_executor.h"
#include "../ai/ai_settings_dialog.h"
#include "../../core/io/importer.h"
#include "../../core/io/project_file.h"
#include "../../core/effects/effect_registry.h"
#include "../../core/templates/essential_graphics.h"
#include "../../core/preferences/preferences.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QStatusBar>
#include <QApplication>
#include <QVBoxLayout>
#include <QSettings>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMimeData>
#include <QClipboard>

namespace FreeEffect {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_project(std::make_unique<ProjectState>())
    , m_commandStack(std::make_unique<CommandStack>(32)) {
    
    setWindowTitle("FreeEffect - Untitled Project");
    setMinimumSize(1280, 720);
    setWindowIcon(QIcon(":/app/icon.svg"));
    setAcceptDrops(true);
    
    setupUi();
    setupMenuBar();
    setupToolBar();
    setupDockWidgets();
    setupStatusBar();
    connectSignals();
    connectMenuActions();
    setupKeyboardShortcuts();
    applyAELayout();
    
    resize(1600, 900);
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUi() {
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
}

void MainWindow::setupMenuBar() {
    m_menuBar = new MenuBar(this);
    setMenuBar(m_menuBar);
}

void MainWindow::setupToolBar() {
    m_toolBar = new QToolBar("Tools", this);
    m_toolBar->setObjectName("ToolsBar");
    m_toolBar->setMovable(false);
    m_toolBar->setIconSize(QSize(20, 20));
    m_toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_toolBar->setStyleSheet(
        "QToolBar { background-color: #3c3c3c; border: none; padding: 1px; spacing: 1px; }"
    );
    
    m_toolGroup = new QActionGroup(this);
    m_toolGroup->setExclusive(true);
    
    auto addToolAction = [this](const QString& icon, const QString& tooltip, 
                               ShortcutAction shortcut, const QString& toolName) {
        QAction* action = m_toolBar->addAction(QIcon(icon), tooltip);
        action->setShortcut(ShortcutManager::instance().getShortcut(shortcut));
        action->setToolTip(tooltip + " (" + ShortcutManager::getShortcutString(shortcut) + ")");
        action->setCheckable(true);
        action->setData(toolName);
        m_toolGroup->addAction(action);
        if (toolName == "selection") action->setChecked(true);
        return action;
    };
    
    addToolAction(":/icons/tools/selection.svg", "Selection Tool (V)", ShortcutAction::ToolSelection, "selection");
    addToolAction(":/icons/tools/hand.svg", "Hand Tool (H)", ShortcutAction::ToolHand, "hand");
    addToolAction(":/icons/tools/zoom.svg", "Zoom Tool (Z)", ShortcutAction::ToolZoom, "zoom");
    addToolAction(":/icons/tools/rotation.svg", "Rotation Tool (W)", ShortcutAction::ToolRotation, "rotation");
    addToolAction(":/icons/tools/anchor.svg", "Pan Behind (Anchor Point) Tool (Y)", ShortcutAction::ShowAnchorPoint, "anchor");
    addToolAction(":/icons/tools/shape.svg", "Shape Tool (Q)", ShortcutAction::ToolPen, "shape");
    addToolAction(":/icons/tools/pen.svg", "Pen Tool (G)", ShortcutAction::ToolPen, "pen");
    addToolAction(":/icons/tools/text.svg", "Horizontal Type Tool (Ctrl+T)", ShortcutAction::ToolText, "text");
    
    addToolBar(m_toolBar);
}

void MainWindow::setupDockWidgets() {
    // Project Panel - Left side
    m_projectPanel = new ProjectPanel(this);
    m_projectDock = new QDockWidget("Project", this);
    m_projectDock->setObjectName("ProjectPanel");
    m_projectDock->setWidget(m_projectPanel);
    m_projectDock->setMinimumWidth(250);
    m_projectDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::LeftDockWidgetArea, m_projectDock);
    
    // Composition Panel - Top center (the canvas)
    m_compositionPanel = new CompositionPanel(this);
    m_compositionDock = new QDockWidget("Composition: (none)", this);
    m_compositionDock->setObjectName("CompositionPanel");
    m_compositionDock->setWidget(m_compositionPanel);
    m_compositionDock->setMinimumWidth(400);
    m_compositionDock->setMinimumHeight(300);
    m_compositionDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::TopDockWidgetArea, m_compositionDock);
    
    // Effect Controls Panel - Right side
    m_effectControlsPanel = new EffectControlsPanel(this);
    m_effectControlsDock = new QDockWidget("Effect Controls: (none)", this);
    m_effectControlsDock->setObjectName("EffectControlsPanel");
    m_effectControlsDock->setWidget(m_effectControlsPanel);
    m_effectControlsDock->setMinimumWidth(250);
    m_effectControlsDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::RightDockWidgetArea, m_effectControlsDock);
    
    // Effects Browser Panel - Left side (tabified with Project panel)
    m_effectsBrowserPanel = new EffectsBrowserPanel(this);
    m_effectsBrowserDock = new QDockWidget("Effects", this);
    m_effectsBrowserDock->setObjectName("EffectsBrowserPanel");
    m_effectsBrowserDock->setWidget(m_effectsBrowserPanel);
    m_effectsBrowserDock->setMinimumWidth(220);
    m_effectsBrowserDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::LeftDockWidgetArea, m_effectsBrowserDock);
    
    // Timeline Panel - Bottom
    m_timelinePanel = new TimelinePanel(this);
    m_timelineDock = new QDockWidget("Timeline: (none)", this);
    m_timelineDock->setObjectName("TimelinePanel");
    m_timelineDock->setWidget(m_timelinePanel);
    m_timelineDock->setMinimumHeight(200);
    m_timelineDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::BottomDockWidgetArea, m_timelineDock);
    
    // AI Assistant Panel - Right side (tabified with Effect Controls)
    m_aiPanel = new AIPanel(this);
    m_aiDock = new QDockWidget("AI Assistant", this);
    m_aiDock->setObjectName("AIPanel");
    m_aiDock->setWidget(m_aiPanel);
    m_aiDock->setMinimumWidth(280);
    m_aiDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::RightDockWidgetArea, m_aiDock);
    
    // Tabify Project and Effect Controls (like AE)
    tabifyDockWidget(m_projectDock, m_effectsBrowserDock);
    tabifyDockWidget(m_effectsBrowserDock, m_effectControlsDock);
    tabifyDockWidget(m_effectControlsDock, m_aiDock);
    m_projectDock->raise();
    
    // Info Panel - Right side (tabified with AI panel)
    m_infoPanel = new InfoPanel(this);
    m_infoDock = new QDockWidget("Info", this);
    m_infoDock->setObjectName("InfoPanel");
    m_infoDock->setWidget(m_infoPanel);
    m_infoDock->setMinimumWidth(200);
    m_infoDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::RightDockWidgetArea, m_infoDock);
    
    // Audio Panel - Right side (tabified)
    m_audioPanel = new AudioPanel(this);
    m_audioDock = new QDockWidget("Audio", this);
    m_audioDock->setObjectName("AudioPanel");
    m_audioDock->setWidget(m_audioPanel);
    m_audioDock->setMinimumWidth(200);
    m_audioDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::RightDockWidgetArea, m_audioDock);
    
    // Preview Panel - Right side (tabified)
    m_previewPanel = new PreviewPanel(this);
    m_previewDock = new QDockWidget("Preview", this);
    m_previewDock->setObjectName("PreviewPanel");
    m_previewDock->setWidget(m_previewPanel);
    m_previewDock->setMinimumWidth(250);
    m_previewDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::RightDockWidgetArea, m_previewDock);
    
    // Character Panel - Right side (tabified)
    m_characterPanel = new CharacterPanel(this);
    m_characterDock = new QDockWidget("Character", this);
    m_characterDock->setObjectName("CharacterPanel");
    m_characterDock->setWidget(m_characterPanel);
    m_characterDock->setMinimumWidth(220);
    m_characterDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::RightDockWidgetArea, m_characterDock);
    
    // Paragraph Panel - Right side (tabified)
    m_paragraphPanel = new ParagraphPanel(this);
    m_paragraphDock = new QDockWidget("Paragraph", this);
    m_paragraphDock->setObjectName("ParagraphPanel");
    m_paragraphDock->setWidget(m_paragraphPanel);
    m_paragraphDock->setMinimumWidth(220);
    m_paragraphDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::RightDockWidgetArea, m_paragraphDock);
    
    // Brushes Panel - Right side (tabified)
    m_brushesPanel = new BrushesPanel(this);
    m_brushesDock = new QDockWidget("Brushes", this);
    m_brushesDock->setObjectName("BrushesPanel");
    m_brushesDock->setWidget(m_brushesPanel);
    m_brushesDock->setMinimumWidth(220);
    m_brushesDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::RightDockWidgetArea, m_brushesDock);
    
    // Paint Panel - Right side (tabified)
    m_paintPanel = new PaintPanel(this);
    m_paintDock = new QDockWidget("Paint", this);
    m_paintDock->setObjectName("PaintPanel");
    m_paintDock->setWidget(m_paintPanel);
    m_paintDock->setMinimumWidth(200);
    m_paintDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::RightDockWidgetArea, m_paintDock);
    
    // Smoother Panel - Right side (tabified)
    m_smootherPanel = new SmootherPanel(this);
    m_smootherDock = new QDockWidget("Smoother", this);
    m_smootherDock->setObjectName("SmootherPanel");
    m_smootherDock->setWidget(m_smootherPanel);
    m_smootherDock->setMinimumWidth(220);
    m_smootherDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::RightDockWidgetArea, m_smootherDock);
    
    // Motion Sketch Panel - Right side (tabified)
    m_motionSketchPanel = new MotionSketchPanel(this);
    m_motionSketchDock = new QDockWidget("Motion Sketch", this);
    m_motionSketchDock->setObjectName("MotionSketchPanel");
    m_motionSketchDock->setWidget(m_motionSketchPanel);
    m_motionSketchDock->setMinimumWidth(220);
    m_motionSketchDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::RightDockWidgetArea, m_motionSketchDock);
    
    // Wiggler Panel - Right side (tabified)
    m_wigglerPanel = new WigglerPanel(this);
    m_wigglerDock = new QDockWidget("Wiggler", this);
    m_wigglerDock->setObjectName("WigglerPanel");
    m_wigglerDock->setWidget(m_wigglerPanel);
    m_wigglerDock->setMinimumWidth(220);
    m_wigglerDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::RightDockWidgetArea, m_wigglerDock);
    
    // Posterize Time Panel - Right side (tabified)
    m_posterizeTimePanel = new PosterizeTimePanel(this);
    m_posterizeTimeDock = new QDockWidget("Posterize Time", this);
    m_posterizeTimeDock->setObjectName("PosterizeTimePanel");
    m_posterizeTimeDock->setWidget(m_posterizeTimePanel);
    m_posterizeTimeDock->setMinimumWidth(220);
    m_posterizeTimeDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::RightDockWidgetArea, m_posterizeTimeDock);
    
    // Tabify all right-side panels together
    tabifyDockWidget(m_aiDock, m_infoDock);
    tabifyDockWidget(m_infoDock, m_audioDock);
    tabifyDockWidget(m_audioDock, m_previewDock);
    tabifyDockWidget(m_previewDock, m_characterDock);
    tabifyDockWidget(m_characterDock, m_paragraphDock);
    tabifyDockWidget(m_paragraphDock, m_brushesDock);
    tabifyDockWidget(m_brushesDock, m_paintDock);
    tabifyDockWidget(m_paintDock, m_smootherDock);
    tabifyDockWidget(m_smootherDock, m_motionSketchDock);
    tabifyDockWidget(m_motionSketchDock, m_wigglerDock);
    tabifyDockWidget(m_wigglerDock, m_posterizeTimeDock);
    
    // Hide most panels by default - only show Info and Character
    m_audioDock->hide();
    m_previewDock->hide();
    m_paragraphDock->hide();
    m_brushesDock->hide();
    m_paintDock->hide();
    m_smootherDock->hide();
    m_motionSketchDock->hide();
    m_wigglerDock->hide();
    m_posterizeTimeDock->hide();
}

void MainWindow::setupStatusBar() {
    statusBar()->setStyleSheet("background-color: #2d2d2d; color: #888888; border-top: 1px solid #1a1a1a; font-size: 11px; padding: 2px 8px;");
    statusBar()->showMessage("Ready");
}

void MainWindow::applyAELayout() {
    resize(1600, 900);
    
    if (m_projectDock) m_projectDock->setMinimumWidth(220);
    if (m_effectsBrowserDock) m_effectsBrowserDock->setMinimumWidth(220);
    if (m_effectControlsDock) m_effectControlsDock->setMinimumWidth(220);
    if (m_compositionDock) m_compositionDock->setMinimumHeight(300);
    if (m_timelineDock) m_timelineDock->setMinimumHeight(200);
}

void MainWindow::connectSignals() {
    // Connect timeline signals
    if (m_timelinePanel) {
        connect(m_timelinePanel, &TimelinePanel::layerSelected, this, &MainWindow::selectLayer);
    }
    
    // Connect project panel signals
    if (m_projectPanel) {
        connect(m_projectPanel, &ProjectPanel::assetDoubleClicked, this, [this](const UUID& id) {
            auto comp = m_project->getCompositionById(id);
            if (comp) {
                if (m_compositionPanel) m_compositionPanel->setComposition(comp);
                if (m_timelinePanel) m_timelinePanel->setComposition(comp, m_commandStack.get());
                m_compositionDock->setWindowTitle(QString("Composition: %1").arg(QString::fromStdString(comp->getName())));
                m_timelineDock->setWindowTitle(QString("Timeline: %1").arg(QString::fromStdString(comp->getName())));
            }
        });
    }
    
    // Connect canvas file drop for import
    if (CanvasWidget* canvas = getCanvasWidget()) {
        connect(canvas, &CanvasWidget::fileDropped, this, [this](const QString& filePath) {
            Importer importer(m_project.get());
            auto asset = importer.importFile(filePath.toStdString());
            if (asset) {
                m_project->setModified(true);
                updateTitle();
                refreshAllPanels();
                statusBar()->showMessage("Imported: " + filePath, 3000);
            }
        });
    }
    
    // Connect tool group for tool switching
    if (m_toolGroup) {
        connect(m_toolGroup, &QActionGroup::triggered, this, [this](QAction* action) {
            if (action) {
                setTool(action->data().toString());
            }
        });
    }
    
    // Connect effects browser panel
    if (m_effectsBrowserPanel) {
        connect(m_effectsBrowserPanel, &EffectsBrowserPanel::effectRequested, 
                this, [this](const QString& effectName) {
            if (m_selectedLayerIndex < 0) {
                statusBar()->showMessage("No layer selected to apply effect", 3000);
                return;
            }
            
            if (m_compositionPanel && m_compositionPanel->getComposition()) {
                auto layers = m_compositionPanel->getComposition()->getLayers();
                if (m_selectedLayerIndex < static_cast<int>(layers.size())) {
                    auto layer = layers[m_selectedLayerIndex];
                    auto& registry = EffectRegistry::instance();
                    if (registry.hasEffect(effectName.toStdString())) {
                        auto effect = registry.create(effectName.toStdString());
                        if (effect) {
                            layer->addEffect(std::shared_ptr<Effect>(std::move(effect)));
                            if (m_effectControlsPanel) {
                                m_effectControlsPanel->refreshControls();
                            }
                            statusBar()->showMessage(
                                QString("Applied effect: %1 to %2")
                                    .arg(effectName)
                                    .arg(QString::fromStdString(layer->getName())), 
                                3000);
                        }
                    }
                }
            }
        });
    }
    
    // Connect Info panel mouse tracking
    if (m_infoPanel) {
        connect(m_infoPanel, &InfoPanel::positionChanged, this, [this](int x, int y) {
            statusBar()->showMessage(QString("Cursor: %1, %2").arg(x).arg(y), 1000);
        });
    }
    
    // Connect Preview panel controls
    if (m_previewPanel) {
        connect(m_previewPanel, &PreviewPanel::playClicked, this, [this]() {
            statusBar()->showMessage("Playing", 1000);
        });
        connect(m_previewPanel, &PreviewPanel::pauseClicked, this, [this]() {
            statusBar()->showMessage("Paused", 1000);
        });
        connect(m_previewPanel, &PreviewPanel::stopClicked, this, [this]() {
            statusBar()->showMessage("Stopped", 1000);
        });
    }
    
    // Connect Audio panel
    if (m_audioPanel) {
        connect(m_audioPanel, &AudioPanel::volumeChanged, this, [this](double vol) {
            statusBar()->showMessage(QString("Volume: %1%").arg(static_cast<int>(vol * 100)), 1000);
        });
    }
}

void MainWindow::connectMenuActions() {
    // Find and connect all menu actions
    auto connectAction = [this](const QString& name, auto slot) {
        QAction* action = findChild<QAction*>(name);
        if (action) connect(action, &QAction::triggered, this, slot);
    };
    
    // File menu
    connectAction("actionNewProject", [this]() {
        auto* newWindow = new MainWindow();
        newWindow->show();
    });
    connectAction("actionOpenProject", [this]() {
        QString file = QFileDialog::getOpenFileName(this, "Open Project", QString(),
            "FreeEffect Projects (*.feproj);;All Files (*)");
        if (!file.isEmpty()) onOpenProject(file);
    });
    connectAction("actionSave", &MainWindow::onSaveProject);
    connectAction("actionSaveAs", &MainWindow::onSaveProjectAs);
    connectAction("actionImport", &MainWindow::onImportFile);
    connectAction("actionNewComposition", &MainWindow::onNewComposition);
    connectAction("actionAddToRenderQueue", &MainWindow::onAddToRenderQueue);
    
    // Composition menu
    connectAction("actionCompSettings", [this]() {
        if (!m_compositionPanel || !m_compositionPanel->getComposition()) return;
        auto comp = m_compositionPanel->getComposition();
        NewCompositionDialog dialog(this);
        dialog.setWindowTitle("Composition Settings");
        dialog.setCompositionName(QString::fromStdString(comp->getName()));
        dialog.setWidth(comp->getResolution().width);
        dialog.setHeight(comp->getResolution().height);
        dialog.setFrameRate(comp->getFrameRate().fps);
        dialog.setDuration(comp->getDuration());
        if (dialog.exec() == QDialog::Accepted) {
            comp->setName(dialog.getCompositionName().toStdString());
            comp->setResolution({dialog.getWidth(), dialog.getHeight()});
            comp->setFrameRate({dialog.getFrameRate()});
            comp->setDuration(dialog.getDuration());
            m_project->setModified(true);
            updateTitle();
            refreshAllPanels();
            m_compositionDock->setWindowTitle(
                QString("Composition: %1").arg(dialog.getCompositionName()));
            m_timelineDock->setWindowTitle(
                QString("Timeline: %1").arg(dialog.getCompositionName()));
        }
    });
    connectAction("actionEssentialGraphics", [this]() {
        if (!m_compositionPanel || !m_compositionPanel->getComposition()) {
            statusBar()->showMessage("No composition open", 3000);
            return;
        }
        EssentialGraphics eg;
        eg.setCompName(m_compositionPanel->getComposition()->getName());
        QString file = QFileDialog::getSaveFileName(this, "Export Essential Graphics Template",
            QString(), "FreeEffect Templates (*.femgt);;All Files (*)");
        if (!file.isEmpty()) {
            if (eg.exportMotionGraphicTemplate(file.toStdString())) {
                statusBar()->showMessage("Exported essential graphics template", 3000);
            } else {
                QMessageBox::warning(this, "Export Error", "Could not export template.");
            }
        }
    });
    
    // Edit menu
    connectAction("actionUndo", &MainWindow::onUndo);
    connectAction("actionRedo", &MainWindow::onRedo);
    connectAction("actionCut", &MainWindow::onCut);
    connectAction("actionCopy", &MainWindow::onCopy);
    connectAction("actionPaste", &MainWindow::onPaste);
    connectAction("actionSelectAll", &MainWindow::onSelectAll);
    connectAction("actionDeselectAll", &MainWindow::onDeselectAll);
    
    // Layer menu
    connectAction("actionNewSolid", &MainWindow::onNewSolidLayer);
    connectAction("actionNewText", &MainWindow::onNewTextLayer);
    connectAction("actionNewNull", &MainWindow::onNewNullLayer);
    connectAction("actionNewAdjustment", &MainWindow::onNewAdjustmentLayer);
    connectAction("actionDeleteLayer", &MainWindow::onDeleteSelectedLayer);
    connectAction("actionDuplicateLayer", &MainWindow::onDuplicateLayer);
    
    // View menu
    connectAction("actionZoomIn", &MainWindow::onZoomIn);
    connectAction("actionZoomOut", &MainWindow::onZoomOut);
    connectAction("actionFitToWindow", &MainWindow::onFitToWindow);
    connectAction("actionToggleGrid", &MainWindow::onToggleGrid);
    connectAction("actionToggleRulers", &MainWindow::onToggleRulers);
    
    // Window menu
    connectAction("actionPreferences", &MainWindow::onShowPreferences);
    connectAction("actionAbout", &MainWindow::onShowAbout);
    connectAction("actionAISettings", &MainWindow::onShowAISettings);
    connectAction("actionToggleAI", &MainWindow::onToggleAIPanel);
    
    // Window panel toggles
    connectAction("actionToggleInfo", [this]() {
        if (m_infoDock) { m_infoDock->isVisible() ? m_infoDock->hide() : (m_infoDock->show(), m_infoDock->raise()); }
    });
    connectAction("actionToggleAudio", [this]() {
        if (m_audioDock) { m_audioDock->isVisible() ? m_audioDock->hide() : (m_audioDock->show(), m_audioDock->raise()); }
    });
    connectAction("actionTogglePreview", [this]() {
        if (m_previewDock) { m_previewDock->isVisible() ? m_previewDock->hide() : (m_previewDock->show(), m_previewDock->raise()); }
    });
    connectAction("actionToggleCharacter", [this]() {
        if (m_characterDock) { m_characterDock->isVisible() ? m_characterDock->hide() : (m_characterDock->show(), m_characterDock->raise()); }
    });
    connectAction("actionToggleParagraph", [this]() {
        if (m_paragraphDock) { m_paragraphDock->isVisible() ? m_paragraphDock->hide() : (m_paragraphDock->show(), m_paragraphDock->raise()); }
    });
    connectAction("actionToggleBrushes", [this]() {
        if (m_brushesDock) { m_brushesDock->isVisible() ? m_brushesDock->hide() : (m_brushesDock->show(), m_brushesDock->raise()); }
    });
    connectAction("actionTogglePaint", [this]() {
        if (m_paintDock) { m_paintDock->isVisible() ? m_paintDock->hide() : (m_paintDock->show(), m_paintDock->raise()); }
    });
    connectAction("actionToggleSmoother", [this]() {
        if (m_smootherDock) { m_smootherDock->isVisible() ? m_smootherDock->hide() : (m_smootherDock->show(), m_smootherDock->raise()); }
    });
    connectAction("actionToggleMotionSketch", [this]() {
        if (m_motionSketchDock) { m_motionSketchDock->isVisible() ? m_motionSketchDock->hide() : (m_motionSketchDock->show(), m_motionSketchDock->raise()); }
    });
    connectAction("actionToggleWiggler", [this]() {
        if (m_wigglerDock) { m_wigglerDock->isVisible() ? m_wigglerDock->hide() : (m_wigglerDock->show(), m_wigglerDock->raise()); }
    });
    connectAction("actionTogglePosterizeTime", [this]() {
        if (m_posterizeTimeDock) { m_posterizeTimeDock->isVisible() ? m_posterizeTimeDock->hide() : (m_posterizeTimeDock->show(), m_posterizeTimeDock->raise()); }
    });
    
    // Effect menu - connect all dynamically created effect actions
    if (m_menuBar && m_menuBar->getEffectMenu()) {
        QList<QAction*> effectActions = m_menuBar->getEffectMenu()->findChildren<QAction*>();
        for (QAction* action : effectActions) {
            QString effectName = action->text();
            connect(action, &QAction::triggered, this, [this, effectName]() {
                if (m_effectControlsPanel) {
                    m_effectControlsPanel->addEffectToSelectedLayer(effectName);
                }
            });
        }
    }
}

void MainWindow::setupKeyboardShortcuts() {
    auto& sm = ShortcutManager::instance();
    
    // Apply all shortcuts to menu actions
    QList<QAction*> allActions = findChildren<QAction*>();
    for (QAction* action : allActions) {
        QString actionName = action->objectName();
        if (actionName == "actionUndo") action->setShortcut(sm.getShortcut(ShortcutAction::Undo));
        else if (actionName == "actionRedo") action->setShortcut(sm.getShortcut(ShortcutAction::Redo));
        else if (actionName == "actionSave") action->setShortcut(sm.getShortcut(ShortcutAction::Save));
        else if (actionName == "actionSaveAs") action->setShortcut(sm.getShortcut(ShortcutAction::SaveAs));
        else if (actionName == "actionNewComposition") action->setShortcut(sm.getShortcut(ShortcutAction::NewComposition));
        else if (actionName == "actionImport") action->setShortcut(sm.getShortcut(ShortcutAction::ImportFile));
        else if (actionName == "actionAddToRenderQueue") action->setShortcut(sm.getShortcut(ShortcutAction::AddToRenderQueue));
    }
}

void MainWindow::updateTitle() {
    QString title = "FreeEffect";
    if (!m_project->getFilePath().empty()) {
        title += " - " + QString::fromStdString(m_project->getFilePath());
    } else {
        title += " - Untitled Project";
    }
    if (m_project->isModified()) title += " *";
    setWindowTitle(title);
}

void MainWindow::refreshAllPanels() {
    if (m_projectPanel) m_projectPanel->refreshAssetList();
    if (m_effectControlsPanel) m_effectControlsPanel->refreshControls();
}

void MainWindow::setTool(const QString& toolName) {
    statusBar()->showMessage(toolName + " Tool", 2000);
    // Update canvas cursor based on tool
    if (CanvasWidget* canvas = getCanvasWidget()) {
        if (toolName == "hand") {
            canvas->setCursor(Qt::OpenHandCursor);
        } else if (toolName == "zoom") {
            canvas->setCursor(Qt::CrossCursor);
        } else if (toolName == "rotation") {
            canvas->setCursor(Qt::SizeAllCursor);
        } else if (toolName == "text" || toolName == "pen" || toolName == "shape") {
            canvas->setCursor(Qt::CrossCursor);
        } else {
            canvas->setCursor(Qt::ArrowCursor);
        }
    }
}

void MainWindow::selectLayer(int index) {
    m_selectedLayerIndex = index;
    if (m_compositionPanel && m_compositionPanel->getComposition()) {
        auto layers = m_compositionPanel->getComposition()->getLayers();
        if (index >= 0 && index < static_cast<int>(layers.size())) {
            if (m_effectControlsPanel) m_effectControlsPanel->setLayer(layers[index]);
            m_effectControlsDock->setWindowTitle(
                QString("Effect Controls: %1").arg(QString::fromStdString(layers[index]->getName())));
        }
    }
}

void MainWindow::deselectAllLayers() {
    m_selectedLayerIndex = -1;
    if (m_effectControlsPanel) m_effectControlsPanel->setLayer(nullptr);
    m_effectControlsDock->setWindowTitle("Effect Controls: (none)");
}

void MainWindow::onNewComposition() {
    NewCompositionDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        auto comp = m_project->addComposition(
            dialog.getCompositionName().toStdString(),
            {dialog.getWidth(), dialog.getHeight()},
            {dialog.getFrameRate()},
            dialog.getDuration()
        );
        m_project->setModified(true);
        updateTitle();
        refreshAllPanels();
        
        // Auto-open the new composition
        if (comp && m_compositionPanel) {
            m_compositionPanel->setComposition(comp);
            if (m_timelinePanel) m_timelinePanel->setComposition(comp, m_commandStack.get());
            m_compositionDock->setWindowTitle(QString("Composition: %1").arg(dialog.getCompositionName()));
            m_timelineDock->setWindowTitle(QString("Timeline: %1").arg(dialog.getCompositionName()));
        }
    }
}

void MainWindow::onImportFile() {
    QString file = QFileDialog::getOpenFileName(this,
        "Import File", QString(),
        "All Supported (*.mp4 *.mov *.avi *.mkv *.png *.jpg *.jpeg *.bmp *.tiff *.gif *.wav *.mp3 *.ogg);;"
        "Video (*.mp4 *.mov *.avi *.mkv *.webm);;"
        "Images (*.png *.jpg *.jpeg *.bmp *.tiff *.gif);;"
        "Audio (*.wav *.mp3 *.ogg *.flac);;"
        "After Effects Projects (*.aep);;"
        "All Files (*)");
    
    if (!file.isEmpty()) {
        Importer importer(m_project.get());
        auto asset = importer.importFile(file.toStdString());
        if (asset) {
            m_project->setModified(true);
            updateTitle();
            refreshAllPanels();
            statusBar()->showMessage("Imported: " + file, 3000);
        } else {
            QMessageBox::warning(this, "Import Error",
                "Could not import file: " + file + "\nThe file may be corrupted or in an unsupported format.");
        }
    }
}

void MainWindow::onOpenProject(const QString& filePath) {
    QString file = filePath;
    if (file.isEmpty()) {
        file = QFileDialog::getOpenFileName(this, "Open Project", QString(),
            "FreeEffect Projects (*.feproj);;All Files (*)");
        if (file.isEmpty()) return;
    }
    
    ProjectFile pf;
    auto result = pf.load(file.toStdString(), *m_project);
    if (result.success) {
        m_project->setFilePath(file.toStdString());
        updateTitle();
        refreshAllPanels();
        addRecentProject(file);
        statusBar()->showMessage("Opened: " + file, 3000);
    } else {
        QMessageBox::warning(this, "Open Error", 
            "Could not open project: " + file + "\n" + QString::fromStdString(result.errorMessage));
    }
}

void MainWindow::onSaveProject() {
    if (m_project->getFilePath().empty()) {
        onSaveProjectAs();
        return;
    }
    
    ProjectFile pf;
    if (pf.save(*m_project, m_project->getFilePath())) {
        m_project->setModified(false);
        updateTitle();
        statusBar()->showMessage("Project saved", 3000);
    } else {
        QMessageBox::warning(this, "Save Error", "Could not save project.");
    }
}

void MainWindow::onSaveProjectAs() {
    QString file = QFileDialog::getSaveFileName(this, "Save Project As", QString(),
        "FreeEffect Projects (*.feproj)");
    if (file.isEmpty()) return;
    
    ProjectFile pf;
    if (pf.save(*m_project, file.toStdString())) {
        m_project->setFilePath(file.toStdString());
        m_project->setModified(false);
        updateTitle();
        addRecentProject(file);
        statusBar()->showMessage("Project saved as: " + file, 3000);
    } else {
        QMessageBox::warning(this, "Save Error", "Could not save project.");
    }
}

void MainWindow::onAddToRenderQueue() {
    RenderQueueDialog dialog(this);
    if (m_compositionPanel && m_compositionPanel->getComposition()) {
        QString compName = QString::fromStdString(m_compositionPanel->getComposition()->getName());
        dialog.addItem(compName, "/output/" + compName + ".mp4");
    }
    dialog.exec();
}

void MainWindow::onShowPreferences() {
    PreferencesDialog dialog(this);
    dialog.exec();
}

void MainWindow::onShowAbout() {
    AboutDialog dialog(this);
    dialog.exec();
}

void MainWindow::onUndo() {
    if (m_commandStack && m_commandStack->canUndo()) {
        m_commandStack->undo();
        refreshAllPanels();
        statusBar()->showMessage("Undo", 2000);
    }
}

void MainWindow::onRedo() {
    if (m_commandStack && m_commandStack->canRedo()) {
        m_commandStack->redo();
        refreshAllPanels();
        statusBar()->showMessage("Redo", 2000);
    }
}

void MainWindow::onShowAISettings() {
    if (!m_aiPanel) return;
    AISettingsDialog dialog(m_aiPanel->getConnector(), this);
    dialog.exec();
}

void MainWindow::onToggleAIPanel() {
    if (m_aiDock) {
        if (m_aiDock->isVisible()) {
            m_aiDock->hide();
        } else {
            m_aiDock->show();
            m_aiDock->raise();
        }
    }
}

void MainWindow::addRecentProject(const QString& path) {
    QSettings settings("FreeEffect", "FreeEffect");
    QStringList recent = settings.value("recentProjects").toStringList();
    recent.removeAll(path);
    recent.prepend(path);
    while (recent.size() > 10) recent.removeLast();
    settings.setValue("recentProjects", recent);
}

AICommandExecutor* MainWindow::getAICommandExecutor() const {
    if (m_aiPanel) return m_aiPanel->getCommandExecutor();
    return nullptr;
}

CanvasWidget* MainWindow::getCanvasWidget() const {
    if (m_compositionPanel) {
        // Access canvas through composition panel's layout
        if (auto* canvas = m_compositionPanel->findChild<CanvasWidget*>()) {
            return canvas;
        }
    }
    return nullptr;
}

void MainWindow::activateToolByName(const QString& name) {
    if (!m_toolGroup) return;
    for (QAction* action : m_toolGroup->actions()) {
        if (action->data().toString() == name) {
            action->setChecked(true);
            setTool(name);
            break;
        }
    }
}

void MainWindow::onNewSolidLayer() {
    if (!m_compositionPanel || !m_compositionPanel->getComposition()) return;
    auto comp = m_compositionPanel->getComposition();
    auto layer = comp->addLayer("Solid", LayerType::Solid);
    m_project->setModified(true);
    refreshAllPanels();
    if (m_timelinePanel) m_timelinePanel->refreshTimeline();
    statusBar()->showMessage("Added solid layer", 2000);
}

void MainWindow::onNewTextLayer() {
    if (!m_compositionPanel || !m_compositionPanel->getComposition()) return;
    auto comp = m_compositionPanel->getComposition();
    auto layer = comp->addLayer("Text", LayerType::Text);
    m_project->setModified(true);
    refreshAllPanels();
    if (m_timelinePanel) m_timelinePanel->refreshTimeline();
    statusBar()->showMessage("Added text layer", 2000);
}

void MainWindow::onNewNullLayer() {
    if (!m_compositionPanel || !m_compositionPanel->getComposition()) return;
    auto comp = m_compositionPanel->getComposition();
    auto layer = comp->addLayer("Null Object", LayerType::Null);
    m_project->setModified(true);
    refreshAllPanels();
    if (m_timelinePanel) m_timelinePanel->refreshTimeline();
    statusBar()->showMessage("Added null object", 2000);
}

void MainWindow::onNewAdjustmentLayer() {
    if (!m_compositionPanel || !m_compositionPanel->getComposition()) return;
    auto comp = m_compositionPanel->getComposition();
    auto layer = comp->addLayer("Adjustment Layer", LayerType::Solid);
    m_project->setModified(true);
    refreshAllPanels();
    if (m_timelinePanel) m_timelinePanel->refreshTimeline();
    statusBar()->showMessage("Added adjustment layer", 2000);
}

void MainWindow::onDeleteSelectedLayer() {
    if (!m_compositionPanel || !m_compositionPanel->getComposition()) return;
    if (m_selectedLayerIndex < 0) return;
    
    auto comp = m_compositionPanel->getComposition();
    auto layers = comp->getLayers();
    if (m_selectedLayerIndex >= static_cast<int>(layers.size())) return;
    
    comp->removeLayer(layers[m_selectedLayerIndex]->getId());
    m_project->setModified(true);
    deselectAllLayers();
    refreshAllPanels();
    if (m_timelinePanel) m_timelinePanel->refreshTimeline();
    statusBar()->showMessage("Deleted layer", 2000);
}

void MainWindow::onDuplicateLayer() {
    if (!m_compositionPanel || !m_compositionPanel->getComposition()) return;
    if (m_selectedLayerIndex < 0) return;
    
    auto comp = m_compositionPanel->getComposition();
    auto layers = comp->getLayers();
    if (m_selectedLayerIndex >= static_cast<int>(layers.size())) return;
    
    auto src = layers[m_selectedLayerIndex];
    auto dup = comp->addLayer(src->getName() + " Copy", src->getType());
    dup->setStartTime(src->getStartTime());
    dup->setDuration(src->getDuration());
    dup->setVisible(src->isVisible());
    
    m_project->setModified(true);
    refreshAllPanels();
    if (m_timelinePanel) m_timelinePanel->refreshTimeline();
    statusBar()->showMessage("Duplicated layer", 2000);
}

void MainWindow::onCut() {
    onCopy();
    onDeleteSelectedLayer();
}

void MainWindow::onCopy() {
    if (m_selectedLayerIndex < 0) return;
    if (!m_compositionPanel || !m_compositionPanel->getComposition()) return;
    
    auto comp = m_compositionPanel->getComposition();
    auto layers = comp->getLayers();
    if (m_selectedLayerIndex >= static_cast<int>(layers.size())) return;
    
    auto layer = layers[m_selectedLayerIndex];
    QJsonObject obj;
    obj["name"] = QString::fromStdString(layer->getName());
    obj["type"] = static_cast<int>(layer->getType());
    obj["duration"] = layer->getDuration();
    obj["startTime"] = layer->getStartTime();
    
    QMimeData* mime = new QMimeData();
    mime->setData("application/x-freeeffect-layer", QJsonDocument(obj).toJson());
    QApplication::clipboard()->setMimeData(mime);
    statusBar()->showMessage("Copied layer: " + QString::fromStdString(layer->getName()), 2000);
}

void MainWindow::onPaste() {
    const QMimeData* mime = QApplication::clipboard()->mimeData();
    if (!mime || !mime->hasFormat("application/x-freeeffect-layer")) return;
    if (!m_compositionPanel || !m_compositionPanel->getComposition()) return;
    
    QJsonDocument doc = QJsonDocument::fromJson(mime->data("application/x-freeeffect-layer"));
    QJsonObject obj = doc.object();
    
    auto comp = m_compositionPanel->getComposition();
    LayerType type = static_cast<LayerType>(obj["type"].toInt());
    auto layer = comp->addLayer(obj["name"].toString().toStdString(), type);
    layer->setStartTime(obj["startTime"].toDouble());
    layer->setDuration(obj["duration"].toDouble());
    
    m_project->setModified(true);
    updateTitle();
    refreshAllPanels();
    if (m_timelinePanel) m_timelinePanel->refreshTimeline();
    statusBar()->showMessage("Pasted layer: " + obj["name"].toString(), 2000);
}

void MainWindow::onSelectAll() {
    if (m_timelinePanel) {
        // Select all layers in timeline
        m_timelinePanel->refreshTimeline();
    }
    if (m_compositionPanel && m_compositionPanel->getComposition()) {
        auto layers = m_compositionPanel->getComposition()->getLayers();
        if (!layers.empty()) {
            selectLayer(0);
        }
    }
    statusBar()->showMessage("Selected all layers", 1000);
}

void MainWindow::onDeselectAll() {
    deselectAllLayers();
}

void MainWindow::onZoomIn() {
    if (CanvasWidget* canvas = getCanvasWidget()) {
        canvas->zoomIn();
    }
    statusBar()->showMessage("Zoom In", 1000);
}

void MainWindow::onZoomOut() {
    if (CanvasWidget* canvas = getCanvasWidget()) {
        canvas->zoomOut();
    }
    statusBar()->showMessage("Zoom Out", 1000);
}

void MainWindow::onFitToWindow() {
    if (CanvasWidget* canvas = getCanvasWidget()) {
        canvas->fitToWindow();
    }
    if (m_compositionPanel) m_compositionPanel->updateZoomCombo(1.0);
    statusBar()->showMessage("Fit to Window", 1000);
}

void MainWindow::onToggleGrid() {
    if (CanvasWidget* canvas = getCanvasWidget()) {
        canvas->setShowGrid(!canvas->isShowGrid());
    }
    statusBar()->showMessage("Toggle Grid", 1000);
}

void MainWindow::onToggleRulers() {
    if (CanvasWidget* canvas = getCanvasWidget()) {
        canvas->setShowRulers(!canvas->isShowRulers());
    }
    statusBar()->showMessage("Toggle Rulers", 1000);
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event) {
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent* event) {
    const QMimeData* mime = event->mimeData();
    if (mime->hasUrls()) {
        for (const QUrl& url : mime->urls()) {
            if (url.isLocalFile()) {
                QString filePath = url.toLocalFile();
                Importer importer(m_project.get());
                auto asset = importer.importFile(filePath.toStdString());
                if (asset) {
                    m_project->setModified(true);
                    updateTitle();
                    refreshAllPanels();
                    statusBar()->showMessage("Imported: " + filePath, 3000);
                }
            }
        }
    }
}

} // namespace FreeEffect
