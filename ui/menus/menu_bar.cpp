#include "menu_bar.h"
#include "../mainwindow/main_window.h"
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
    m_compositionMenu->addSeparator();
    m_compositionMenu->addAction(createNamedActionWithShortcut("actionAddToRenderQueue", "Add to &Render Queue", ShortcutAction::AddToRenderQueue));
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
    m_layerMenu->addAction(createMenuAction("Layer &Settings..."));
    
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
    m_effectMenu->setEnabled(false);
}

void MenuBar::createAnimationMenu() {
    m_animationMenu = addMenu("A&nimation");
    m_animationMenu->addAction(createMenuAction("&Add Keyframe"));
    m_animationMenu->addAction(createMenuAction("Toggle &Hold Keyframe"));
    m_animationMenu->addAction(createMenuAction("Keyframe &Interpolation..."));
    m_animationMenu->addSeparator();
    m_animationMenu->addAction(createMenuActionWithShortcut("Reveal &Animated Properties", ShortcutAction::RevealAllAnimated));
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
}

void MenuBar::createWindowMenu() {
    m_windowMenu = addMenu("&Window");
    m_windowMenu->addAction(createMenuAction("Project"));
    m_windowMenu->addAction(createMenuAction("Composition"));
    m_windowMenu->addAction(createMenuAction("Timeline"));
    m_windowMenu->addAction(createMenuAction("Effect Controls"));
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

} // namespace FreeEffect
