#include "menu_bar.h"
#include "../mainwindow/main_window.h"
#include "../../core/effects/effect_registry.h"
#include <QFileDialog>
#include <QMessageBox>

namespace FreeEffect {

MenuBar::MenuBar(MainWindow* parent)
    : QMenuBar(parent)
    , m_mainWindow(parent) {
    createFileMenu();
    createEditMenu();
    createCompositionMenu();
    createLayerMenu();
    createEffectMenu();
    createAnimationMenu();
    createViewMenu();
    createWindowMenu();
    createScriptMenu();
    createHelpMenu();
}

QAction* MenuBar::createNamedAction(const QString& name, const QString& text, QMenu* parent) {
    QAction* action = new QAction(text, this);
    action->setObjectName(name);
    if (parent) parent->addAction(action);
    return action;
}

QAction* MenuBar::createNamedActionWithShortcut(const QString& name, const QString& text, 
                                                 ShortcutAction shortcut, QMenu* parent) {
    QAction* action = new QAction(text, this);
    action->setObjectName(name);
    action->setShortcut(ShortcutManager::instance().getShortcut(shortcut));
    if (parent) parent->addAction(action);
    return action;
}

QAction* MenuBar::createMenuAction(const QString& text, QMenu* parent) {
    QAction* action = new QAction(text, this);
    if (parent) parent->addAction(action);
    return action;
}

QAction* MenuBar::createMenuActionWithShortcut(const QString& text, ShortcutAction shortcut, QMenu* parent) {
    QAction* action = new QAction(text, this);
    action->setShortcut(ShortcutManager::instance().getShortcut(shortcut));
    if (parent) parent->addAction(action);
    return action;
}

void MenuBar::createFileMenu() {
    m_fileMenu = addMenu("&File");
    
    QMenu* newMenu = m_fileMenu->addMenu("&New");
    newMenu->addAction(createNamedActionWithShortcut("actionNewProject", "New Project", ShortcutAction::NewProject));
    
    m_fileMenu->addAction(createNamedActionWithShortcut("actionOpenProject", "&Open Project...", ShortcutAction::OpenProject));
    
    QMenu* recentMenu = m_fileMenu->addMenu("Open &Recent");
    recentMenu->setEnabled(false);
    
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(createNamedActionWithShortcut("actionSave", "&Save", ShortcutAction::Save));
    m_fileMenu->addAction(createNamedActionWithShortcut("actionSaveAs", "Save &As...", ShortcutAction::SaveAs));
    m_fileMenu->addAction(createMenuAction("Save a Copy"));
    m_fileMenu->addAction(createMenuAction("&Revert"));
    
    m_fileMenu->addSeparator();
    
    QMenu* importMenu = m_fileMenu->addMenu("&Import");
    importMenu->addAction(createNamedActionWithShortcut("actionImport", "File...", ShortcutAction::ImportFile));
    
    QMenu* exportMenu = m_fileMenu->addMenu("&Export");
    exportMenu->addAction(createNamedActionWithShortcut("actionAddToRenderQueue", "Add to Render Queue", ShortcutAction::AddToRenderQueue));
    
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(createMenuAction("Project Settings..."));
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(createMenuAction("E&xit"));
}

void MenuBar::createEditMenu() {
    m_editMenu = addMenu("&Edit");
    
    m_editMenu->addAction(createNamedActionWithShortcut("actionUndo", "&Undo", ShortcutAction::Undo));
    m_editMenu->addAction(createNamedActionWithShortcut("actionRedo", "&Redo", ShortcutAction::Redo));
    m_editMenu->addSeparator();
    m_editMenu->addAction(createNamedActionWithShortcut("actionCut", "Cu&t", ShortcutAction::Cut));
    m_editMenu->addAction(createNamedActionWithShortcut("actionCopy", "&Copy", ShortcutAction::Copy));
    m_editMenu->addAction(createNamedActionWithShortcut("actionPaste", "&Paste", ShortcutAction::Paste));
    m_editMenu->addSeparator();
    m_editMenu->addAction(createNamedActionWithShortcut("actionDuplicate", "D&uplicate", ShortcutAction::Duplicate));
    m_editMenu->addSeparator();
    m_editMenu->addAction(createNamedActionWithShortcut("actionSelectAll", "Select &All", ShortcutAction::SelectAll));
    m_editMenu->addAction(createNamedActionWithShortcut("actionDeselectAll", "&Deselect All", ShortcutAction::DeselectAll));
    m_editMenu->addSeparator();
    m_editMenu->addAction(createNamedAction("actionAutoTrace", "Auto-trace..."));
    m_editMenu->addAction(createNamedAction("actionConvertToBezier", "Convert to Bezier Path"));
    m_editMenu->addSeparator();
    
    QMenu* purgeMenu = m_editMenu->addMenu("&Purge");
    purgeMenu->addAction(createMenuAction("All Memory & Disk Cache"));
    
    m_editMenu->addSeparator();
    m_editMenu->addAction(createNamedAction("actionPreferences", "Pre&ferences..."));
    m_editMenu->addSeparator();
    m_editMenu->addAction(createNamedAction("actionAISettings", "AI Assistant Settings..."));
}

void MenuBar::createCompositionMenu() {
    m_compositionMenu = addMenu("&Composition");
    m_compositionMenu->addAction(createNamedActionWithShortcut("actionNewComposition", "&New Composition...", ShortcutAction::NewComposition));
    m_compositionMenu->addAction(createNamedActionWithShortcut("actionCompSettings", "Composition &Settings...", ShortcutAction::CompositionSettings));
    m_compositionMenu->addSeparator();
    m_compositionMenu->addAction(createMenuAction("Trim Composition to Work Area"));
    m_compositionMenu->addAction(createMenuAction("Crop Composition to Region of Interest"));
    m_compositionMenu->addSeparator();
    m_compositionMenu->addAction(createMenuAction("Pre-render"));
    m_compositionMenu->addSeparator();
    m_compositionMenu->addAction(createNamedActionWithShortcut("actionAddToRenderQueue", "Add to &Render Queue", ShortcutAction::AddToRenderQueue));
    m_compositionMenu->addSeparator();
    m_compositionMenu->addAction(createNamedAction("actionEssentialGraphics", "Essential Graphics..."));
}

void MenuBar::createLayerMenu() {
    m_layerMenu = addMenu("&Layer");
    
    QMenu* newLayerMenu = m_layerMenu->addMenu("&New");
    newLayerMenu->addAction(createNamedAction("actionNewSolid", "Solid..."));
    newLayerMenu->addAction(createNamedAction("actionNewText", "Text"));
    newLayerMenu->addAction(createNamedAction("actionNewShape", "Shape Layer"));
    newLayerMenu->addAction(createNamedAction("actionNewNull", "Null Object"));
    newLayerMenu->addAction(createNamedAction("actionNewAdjustment", "Adjustment Layer"));
    newLayerMenu->addAction(createNamedAction("actionNewCamera", "Camera..."));
    newLayerMenu->addAction(createNamedAction("actionNewLight", "Light..."));
    
    m_layerMenu->addSeparator();
    m_layerMenu->addAction(createNamedAction("actionDuplicateLayer", "Duplicate Layer"));
    m_layerMenu->addAction(createNamedAction("actionDeleteLayer", "Delete Layer"));
    m_layerMenu->addAction(createNamedAction("actionPrecompose", "Pre-compose..."));
    m_layerMenu->addAction(createNamedAction("actionSplitLayer", "Split Layer"));
    m_layerMenu->addAction(createMenuAction("Layer &Settings..."));
    
    QMenu* timeMenu = m_layerMenu->addMenu("T&ime");
    timeMenu->addAction(createMenuAction("Enable Time Remapping"));
    timeMenu->addSeparator();
    timeMenu->addAction(createMenuAction("Time Reverse Layer"));
    timeMenu->addAction(createMenuAction("Time Stretch..."));
    timeMenu->addAction(createMenuAction("Freeze Frame"));
    
    QMenu* maskMenu = m_layerMenu->addMenu("&Mask");
    maskMenu->addAction(createMenuAction("New Mask"));
    maskMenu->addAction(createMenuAction("Mask Shape..."));
    maskMenu->addAction(createMenuAction("Mask Feather..."));
    maskMenu->addAction(createMenuAction("Mask Opacity..."));
    maskMenu->addAction(createMenuAction("Mask Expansion..."));
    maskMenu->addSeparator();
    maskMenu->addAction(createMenuAction("Lock Mask"));
    maskMenu->addAction(createMenuAction("Invert Mask"));
    
    QMenu* switchesMenu = m_layerMenu->addMenu("S&witches");
    switchesMenu->addAction(createMenuAction("Shy"));
    switchesMenu->addAction(createMenuAction("Collapse Transformations"));
    switchesMenu->addAction(createMenuAction("Quality Best"));
    switchesMenu->addAction(createMenuAction("Quality Draft"));
    switchesMenu->addAction(createMenuAction("Effect On"));
    switchesMenu->addAction(createMenuAction("Frame Blending"));
    switchesMenu->addAction(createMenuAction("Motion Blur"));
    switchesMenu->addAction(createMenuAction("Adjustment Layer"));
    
    QMenu* createMenu = m_layerMenu->addMenu("&Create");
    createMenu->addAction(createMenuAction("Create Masks from Text"));
    createMenu->addAction(createMenuAction("Create Shapes from Text"));
    
    QMenu* transformMenu = m_layerMenu->addMenu("&Transform");
    transformMenu->addAction(createMenuActionWithShortcut("Position", ShortcutAction::ShowPosition));
    transformMenu->addAction(createMenuActionWithShortcut("Scale", ShortcutAction::ShowScale));
    transformMenu->addAction(createMenuActionWithShortcut("Rotation", ShortcutAction::ShowRotation));
    transformMenu->addAction(createMenuActionWithShortcut("Opacity", ShortcutAction::ShowOpacity));
    transformMenu->addAction(createMenuActionWithShortcut("Anchor Point", ShortcutAction::ShowAnchorPoint));
    
    QMenu* qualityMenu = m_layerMenu->addMenu("&Quality");
    qualityMenu->addAction(createMenuAction("&Best"));
    qualityMenu->addAction(createMenuAction("&Draft"));
    qualityMenu->addAction(createMenuAction("&Wireframe"));
}

void MenuBar::createEffectMenu() {
    m_effectMenu = addMenu("&Effect");
    
    // Populate from EffectRegistry
    auto& registry = EffectRegistry::instance();
    auto categories = registry.getCategories();
    
    for (const auto& cat : categories) {
        QMenu* catMenu = m_effectMenu->addMenu(QString::fromStdString(cat));
        auto effects = registry.getEffectNamesInCategory(cat);
        for (const auto& effName : effects) {
            QAction* action = catMenu->addAction(QString::fromStdString(effName));
            action->setObjectName("effect_" + QString::fromStdString(effName));
        }
    }
    
    if (categories.empty()) {
        m_effectMenu->setEnabled(false);
    }
}

void MenuBar::createAnimationMenu() {
    m_animationMenu = addMenu("A&nimation");
    m_animationMenu->addAction(createMenuAction("&Add Keyframe"));
    m_animationMenu->addAction(createMenuAction("Toggle &Hold Keyframe"));
    m_animationMenu->addAction(createMenuAction("Keyframe &Interpolation..."));
    m_animationMenu->addSeparator();
    m_animationMenu->addAction(createMenuAction("Graph &Editor"));
    m_animationMenu->addAction(createMenuAction("&Value Graph"));
    m_animationMenu->addAction(createMenuAction("Sp&eed Graph"));
    m_animationMenu->addSeparator();
    m_animationMenu->addAction(createMenuActionWithShortcut("Reveal &Animated Properties", ShortcutAction::RevealAllAnimated));
    m_animationMenu->addSeparator();
    QMenu* responsiveMenu = m_animationMenu->addMenu("Responsive Design");
    responsiveMenu->addAction(createMenuAction("Time"));
    responsiveMenu->addAction(createMenuAction("Position Pins..."));
}

void MenuBar::createViewMenu() {
    m_viewMenu = addMenu("&View");
    m_viewMenu->addAction(createNamedActionWithShortcut("actionZoomIn", "Zoom &In", ShortcutAction::ZoomInView));
    m_viewMenu->addAction(createNamedActionWithShortcut("actionZoomOut", "Zoom &Out", ShortcutAction::ZoomOutView));
    m_viewMenu->addAction(createNamedActionWithShortcut("actionFitToWindow", "&Fit to Window", ShortcutAction::FitToWindow));
    m_viewMenu->addSeparator();
    m_viewMenu->addAction(createNamedActionWithShortcut("actionToggleGrid", "Show &Grid", ShortcutAction::ToggleGrid));
    m_viewMenu->addAction(createNamedActionWithShortcut("actionToggleRulers", "Show &Rulers", ShortcutAction::ToggleRulers));
    m_viewMenu->addAction(createMenuAction("Show &Guides"));
    m_viewMenu->addAction(createMenuAction("Show Safe Margins"));
    m_viewMenu->addSeparator();
    QMenu* channelMenu = m_viewMenu->addMenu("Channel &Display");
    channelMenu->addAction(createMenuAction("RGB"));
    channelMenu->addAction(createMenuAction("Red"));
    channelMenu->addAction(createMenuAction("Green"));
    channelMenu->addAction(createMenuAction("Blue"));
    channelMenu->addAction(createMenuAction("Alpha"));
    QMenu* multiViewMenu = m_viewMenu->addMenu("&Multi-View");
    multiViewMenu->addAction(createMenuAction("2 Views - Horizontal"));
    multiViewMenu->addAction(createMenuAction("2 Views - Vertical"));
    multiViewMenu->addAction(createMenuAction("4 Views"));
    QMenu* resMenu = m_viewMenu->addMenu("&Resolution");
    resMenu->addAction(createMenuAction("Full"));
    resMenu->addAction(createMenuAction("Half"));
    resMenu->addAction(createMenuAction("Third"));
    resMenu->addAction(createMenuAction("Quarter"));
}

void MenuBar::createWindowMenu() {
    m_windowMenu = addMenu("&Window");
    m_windowMenu->addAction(createNamedAction("actionToggleInfo", "Info"));
    m_windowMenu->addAction(createNamedAction("actionToggleAudio", "Audio"));
    m_windowMenu->addAction(createNamedAction("actionTogglePreview", "Preview"));
    m_windowMenu->addAction(createNamedAction("actionToggleCharacter", "Character"));
    m_windowMenu->addAction(createNamedAction("actionToggleParagraph", "Paragraph"));
    m_windowMenu->addAction(createNamedAction("actionToggleBrushes", "Brushes"));
    m_windowMenu->addAction(createNamedAction("actionTogglePaint", "Paint"));
    m_windowMenu->addAction(createNamedAction("actionToggleSmoother", "Smoother"));
    m_windowMenu->addAction(createNamedAction("actionToggleMotionSketch", "Motion Sketch"));
    m_windowMenu->addAction(createNamedAction("actionToggleWiggler", "Wiggler"));
    m_windowMenu->addAction(createNamedAction("actionTogglePosterizeTime", "Posterize Time"));
    m_windowMenu->addSeparator();
    m_windowMenu->addAction(createNamedAction("actionToggleAlign", "Align"));
    m_windowMenu->addAction(createNamedAction("actionToggleTracker", "Tracker"));
    m_windowMenu->addAction(createNamedAction("actionToggleContentAwareFill", "Content-Aware Fill"));
    m_windowMenu->addAction(createNamedAction("actionToggleRotoBrush", "Roto Brush"));
    m_windowMenu->addAction(createNamedAction("actionTogglePlanarEditor", "Planar Editor"));
    m_windowMenu->addAction(createNamedAction("actionToggleDataDriven", "Data-Driven"));
    m_windowMenu->addAction(createNamedAction("actionToggleEssentialGraphics", "Essential Graphics"));
    m_windowMenu->addAction(createNamedAction("actionToggleLibraries", "Libraries"));
    m_windowMenu->addSeparator();
    m_windowMenu->addAction(createNamedAction("actionToggleAI", "AI Assistant"));
    m_windowMenu->addSeparator();
    QMenu* workspaceMenu = m_windowMenu->addMenu("&Workspace");
    workspaceMenu->addAction(createMenuAction("All Panels"));
    workspaceMenu->addAction(createMenuAction("Animation"));
    workspaceMenu->addAction(createMenuAction("Effects"));
    workspaceMenu->addSeparator();
    workspaceMenu->addAction(createMenuAction("Save as New Workspace..."));
    workspaceMenu->addAction(createMenuAction("Reset \"Default\" to Saved Layout"));
}

void MenuBar::createHelpMenu() {
    m_helpMenu = addMenu("&Help");
    m_helpMenu->addAction(createNamedAction("actionAbout", "About FreeEffect"));
    m_helpMenu->addSeparator();
    m_helpMenu->addAction(createMenuAction("FreeEffect Help"));
}

void MenuBar::createScriptMenu() {
    m_scriptMenu = addMenu("&Scripts");
    m_scriptMenu->addAction(createNamedAction("actionRunScript", "Run Script..."));
    m_scriptMenu->addAction(createNamedAction("actionOpenScriptEditor", "Script Editor"));
    m_scriptMenu->addSeparator();
    m_scriptMenu->addAction(createMenuAction("Recent Scripts"));
}

} // namespace FreeEffect
