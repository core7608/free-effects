#include "main_window.h"
#include "../panels/project_panel.h"
#include "../panels/composition_panel.h"
#include "../panels/timeline_panel.h"
#include "../panels/effect_controls_panel.h"
#include "../dialogs/new_composition_dialog.h"
#include "../dialogs/about_dialog.h"
#include "../menus/shortcut_manager.h"
#include "../../core/io/importer.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QStatusBar>
#include <QApplication>
#include <QVBoxLayout>

namespace FreeEffect {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_project(std::make_unique<ProjectState>())
    , m_commandStack(std::make_unique<CommandStack>(32)) {
    
    setWindowTitle("FreeEffect - Untitled Project");
    setMinimumSize(1280, 720);
    
    setupUi();
    setupMenuBar();
    setupToolBar();
    setupDockWidgets();
    setupStatusBar();
    connectSignals();
    setupKeyboardShortcuts();
    
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
    
    auto addToolAction = [this](const QString& icon, const QString& tooltip, ShortcutAction shortcut) {
        QAction* action = m_toolBar->addAction(QIcon(icon), tooltip);
        action->setShortcut(ShortcutManager::instance().getShortcut(shortcut));
        action->setToolTip(tooltip + " (" + ShortcutManager::getShortcutString(shortcut) + ")");
        return action;
    };
    
    addToolAction(":/icons/tool_selection.svg", "Selection Tool (V)", ShortcutAction::ToolSelection);
    addToolAction(":/icons/tool_hand.svg", "Hand Tool (H)", ShortcutAction::ToolHand);
    addToolAction(":/icons/tool_zoom.svg", "Zoom Tool (Z)", ShortcutAction::ToolZoom);
    addToolAction(":/icons/tool_rotation.svg", "Rotation Tool (W)", ShortcutAction::ToolRotation);
    addToolAction(":/icons/tool_anchor.svg", "Pan Behind Tool (Y)", ShortcutAction::ShowAnchorPoint);
    addToolAction(":/icons/tool_shape.svg", "Shape Tool (Q)", ShortcutAction::ToolSelection);
    addToolAction(":/icons/tool_pen.svg", "Pen Tool (G)", ShortcutAction::ToolPen);
    addToolAction(":/icons/tool_text.svg", "Text Tool (Ctrl+T)", ShortcutAction::ToolText);
    
    addToolBar(m_toolBar);
}

void MainWindow::setupDockWidgets() {
    m_projectPanel = new ProjectPanel(this);
    m_projectDock = new QDockWidget("Project", this);
    m_projectDock->setObjectName("ProjectPanel");
    m_projectDock->setWidget(m_projectPanel);
    m_projectDock->setMinimumWidth(250);
    addDockWidget(Qt::LeftDockWidgetArea, m_projectDock);
    
    m_compositionPanel = new CompositionPanel(this);
    m_compositionDock = new QDockWidget("Composition", this);
    m_compositionDock->setObjectName("CompositionPanel");
    m_compositionDock->setWidget(m_compositionPanel);
    m_compositionDock->setMinimumWidth(400);
    addDockWidget(Qt::TopDockWidgetArea, m_compositionDock);
    
    m_effectControlsPanel = new EffectControlsPanel(this);
    m_effectControlsDock = new QDockWidget("Effect Controls", this);
    m_effectControlsDock->setObjectName("EffectControlsPanel");
    m_effectControlsDock->setWidget(m_effectControlsPanel);
    m_effectControlsDock->setMinimumWidth(250);
    addDockWidget(Qt::RightDockWidgetArea, m_effectControlsDock);
    
    m_timelinePanel = new TimelinePanel(this);
    m_timelineDock = new QDockWidget("Timeline", this);
    m_timelineDock->setObjectName("TimelinePanel");
    m_timelineDock->setWidget(m_timelinePanel);
    m_timelineDock->setMinimumHeight(200);
    addDockWidget(Qt::BottomDockWidgetArea, m_timelineDock);
    
    tabifyDockWidget(m_projectDock, m_effectControlsDock);
    m_projectDock->raise();
}

void MainWindow::setupStatusBar() {
    statusBar()->showMessage("Ready");
}

void MainWindow::connectSignals() {
    // Project state changes are handled by direct calls to updateTitle()
    // Will be connected to Qt signals when ProjectState gains QObject support
}

void MainWindow::setupKeyboardShortcuts() {
    auto& sm = ShortcutManager::instance();
    
    auto addAction = [&](QAction* action, ShortcutAction sa) {
        action->setShortcut(sm.getShortcut(sa));
    };
    
    // File menu actions
    if (QAction* a = findChild<QAction*>("actionNewProject")) addAction(a, ShortcutAction::NewProject);
    if (QAction* a = findChild<QAction*>("actionSave")) addAction(a, ShortcutAction::Save);
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
    }
}

void MainWindow::onImportFile() {
    QString file = QFileDialog::getOpenFileName(this,
        "Import File", QString(),
        "All Supported (*.mp4 *.mov *.avi *.mkv *.png *.jpg *.jpeg *.bmp *.wav *.mp3 *.ogg);;"
        "Video (*.mp4 *.mov *.avi *.mkv);;"
        "Images (*.png *.jpg *.jpeg *.bmp);;"
        "Audio (*.wav *.mp3 *.ogg);;"
        "All Files (*)");
    
    if (!file.isEmpty()) {
        Importer importer(m_project.get());
        auto asset = importer.importFile(file.toStdString());
        if (asset) {
            m_project->setModified(true);
            updateTitle();
            statusBar()->showMessage("Imported: " + file, 3000);
        } else {
            QMessageBox::warning(this, "Import Error",
                "Could not import file: " + file + "\nThe file may be corrupted or in an unsupported format.");
        }
    }
}

void MainWindow::onAddToRenderQueue() {
    // Render Queue dialog will be implemented
    QMessageBox::information(this, "Render Queue", "Render Queue: Feature coming soon.");
}

} // namespace FreeEffect
