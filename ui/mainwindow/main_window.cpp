#include "main_window.h"
#include "../panels/project_panel.h"
#include "../panels/composition_panel.h"
#include "../panels/timeline_panel.h"
#include "../panels/effect_controls_panel.h"
#include "../panels/timeline_canvas.h"
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
    tabifyDockWidget(m_projectDock, m_effectControlsDock);
    tabifyDockWidget(m_effectControlsDock, m_aiDock);
    m_projectDock->raise();
}

void MainWindow::setupStatusBar() {
    statusBar()->setStyleSheet("background-color: #2d2d2d; color: #888888; border-top: 1px solid #1a1a1a; font-size: 11px; padding: 2px 8px;");
    statusBar()->showMessage("Ready");
}

void MainWindow::applyAELayout() {
    // Resize to match typical AE layout
    resize(1600, 900);
    
    // Set minimum sizes for panels
    if (m_projectDock) m_projectDock->setMinimumWidth(220);
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
