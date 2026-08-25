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

QAction* MenuBar::createAction(const QString& text, QMenu* parent) {
    QAction* action = new QAction(text, this);
    if (parent) parent->addAction(action);
    return action;
}

QAction* MenuBar::createActionWithShortcut(const QString& text, ShortcutAction shortcut, QMenu* parent) {
    QAction* action = new QAction(text, this);
    action->setShortcut(ShortcutManager::instance().getShortcut(shortcut));
    if (parent) parent->addAction(action);
    return action;
}

void MenuBar::createFileMenu() {
    m_fileMenu = addMenu("&File");
    
    QMenu* newMenu = m_fileMenu->addMenu("&New");
    newMenu->addAction(createActionWithShortcut("New Project", ShortcutAction::NewProject));
    
    m_fileMenu->addAction(createActionWithShortcut("&Open Project...", ShortcutAction::OpenProject));
    
    QMenu* recentMenu = m_fileMenu->addMenu("Open &Recent");
    recentMenu->setEnabled(false);
    
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(createActionWithShortcut("&Save", ShortcutAction::Save));
    m_fileMenu->addAction(createActionWithShortcut("Save &As...", ShortcutAction::SaveAs));
    m_fileMenu->addAction(createAction("Save a Copy"));
    m_fileMenu->addAction(createAction("&Revert"));
    
    m_fileMenu->addSeparator();
    
    QMenu* importMenu = m_fileMenu->addMenu("&Import");
    importMenu->addAction(createActionWithShortcut("File...", ShortcutAction::ImportFile));
    
    QMenu* exportMenu = m_fileMenu->addMenu("&Export");
    exportMenu->addAction(createActionWithShortcut("Add to Render Queue", ShortcutAction::AddToRenderQueue));
    
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(createAction("Project Settings..."));
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(createAction("E&xit"));
}

void MenuBar::createEditMenu() {
    m_editMenu = addMenu("&Edit");
    
    m_editMenu->addAction(createActionWithShortcut("&Undo", ShortcutAction::Undo));
    m_editMenu->addAction(createActionWithShortcut("&Redo", ShortcutAction::Redo));
    m_editMenu->addSeparator();
    m_editMenu->addAction(createActionWithShortcut("Cu&t", ShortcutAction::Cut));
    m_editMenu->addAction(createActionWithShortcut("&Copy", ShortcutAction::Copy));
    m_editMenu->addAction(createActionWithShortcut("&Paste", ShortcutAction::Paste));
    m_editMenu->addSeparator();
    m_editMenu->addAction(createActionWithShortcut("D&uplicate", ShortcutAction::Duplicate));
    m_editMenu->addSeparator();
    m_editMenu->addAction(createActionWithShortcut("Select &All", ShortcutAction::SelectAll));
    m_editMenu->addAction(createActionWithShortcut("&Deselect All", ShortcutAction::DeselectAll));
    m_editMenu->addSeparator();
    
    QMenu* purgeMenu = m_editMenu->addMenu("&Purge");
    purgeMenu->addAction(createAction("All Memory & Disk Cache"));
    
    QMenu* prefsMenu = m_editMenu->addMenu("Pre&ferences");
    prefsMenu->addAction(createAction("General..."));
    prefsMenu->addAction(createAction("Import..."));
    prefsMenu->addAction(createAction("Output..."));
    prefsMenu->addAction(createAction("Grids & Guides..."));
    prefsMenu->addAction(createAction("Labels..."));
    prefsMenu->addAction(createAction("Media & Disk Cache..."));
    prefsMenu->addAction(createAction("Memory & CPU..."));
    prefsMenu->addAction(createAction("Audio Hardware..."));
}

void MenuBar::createCompositionMenu() {
    m_compositionMenu = addMenu("&Composition");
    m_compositionMenu->addAction(createActionWithShortcut("&New Composition...", ShortcutAction::NewComposition));
    m_compositionMenu->addAction(createActionWithShortcut("Composition &Settings...", ShortcutAction::CompositionSettings));
    m_compositionMenu->addSeparator();
    m_compositionMenu->addAction(createAction("Trim Composition to Work Area"));
    m_compositionMenu->addSeparator();
    m_compositionMenu->addAction(createActionWithShortcut("Add to &Render Queue", ShortcutAction::AddToRenderQueue));
}

void MenuBar::createLayerMenu() {
    m_layerMenu = addMenu("&Layer");
    
    QMenu* newLayerMenu = m_layerMenu->addMenu("&New");
    newLayerMenu->addAction(createAction("Solid..."));
    newLayerMenu->addAction(createAction("Text"));
    newLayerMenu->addAction(createAction("Shape Layer"));
    newLayerMenu->addAction(createAction("Null Object"));
    newLayerMenu->addAction(createAction("Adjustment Layer"));
    newLayerMenu->addAction(createAction("Camera..."));
    newLayerMenu->addAction(createAction("Light..."));
    
    m_layerMenu->addSeparator();
    m_layerMenu->addAction(createAction("Layer &Settings..."));
    
    QMenu* transformMenu = m_layerMenu->addMenu("&Transform");
    transformMenu->addAction(createActionWithShortcut("Position", ShortcutAction::ShowPosition));
    transformMenu->addAction(createActionWithShortcut("Scale", ShortcutAction::ShowScale));
    transformMenu->addAction(createActionWithShortcut("Rotation", ShortcutAction::ShowRotation));
    transformMenu->addAction(createActionWithShortcut("Opacity", ShortcutAction::ShowOpacity));
    transformMenu->addAction(createActionWithShortcut("Anchor Point", ShortcutAction::ShowAnchorPoint));
    
    QMenu* qualityMenu = m_layerMenu->addMenu("&Quality");
    qualityMenu->addAction(createAction("&Best"));
    qualityMenu->addAction(createAction("&Draft"));
    qualityMenu->addAction(createAction("&Wireframe"));
}

void MenuBar::createEffectMenu() {
    m_effectMenu = addMenu("&Effect");
    m_effectMenu->setEnabled(false);
}

void MenuBar::createAnimationMenu() {
    m_animationMenu = addMenu("A&nimation");
    m_animationMenu->addAction(createAction("&Add Keyframe"));
    m_animationMenu->addAction(createAction("Toggle &Hold Keyframe"));
    m_animationMenu->addAction(createAction("Keyframe &Interpolation..."));
    m_animationMenu->addSeparator();
    m_animationMenu->addAction(createActionWithShortcut("Reveal &Animated Properties", ShortcutAction::RevealAllAnimated));
}

void MenuBar::createViewMenu() {
    m_viewMenu = addMenu("&View");
    m_viewMenu->addAction(createActionWithShortcut("Zoom &In", ShortcutAction::ZoomInView));
    m_viewMenu->addAction(createActionWithShortcut("Zoom &Out", ShortcutAction::ZoomOutView));
    m_viewMenu->addAction(createActionWithShortcut("&Fit to Window", ShortcutAction::FitToWindow));
    m_viewMenu->addSeparator();
    m_viewMenu->addAction(createActionWithShortcut("Show &Grid", ShortcutAction::ToggleGrid));
    m_viewMenu->addAction(createActionWithShortcut("Show &Rulers", ShortcutAction::ToggleRulers));
    m_viewMenu->addAction(createAction("Show &Guides"));
}

void MenuBar::createWindowMenu() {
    m_windowMenu = addMenu("&Window");
    m_windowMenu->addAction(createAction("Project"));
    m_windowMenu->addAction(createAction("Composition"));
    m_windowMenu->addAction(createAction("Timeline"));
    m_windowMenu->addAction(createAction("Effect Controls"));
    m_windowMenu->addSeparator();
    QMenu* workspaceMenu = m_windowMenu->addMenu("&Workspace");
    workspaceMenu->addAction(createAction("Default"));
    workspaceMenu->addAction(createAction("Save as New Workspace..."));
}

void MenuBar::createHelpMenu() {
    m_helpMenu = addMenu("&Help");
    m_helpMenu->addAction(createAction("About FreeEffect"));
}

} // namespace FreeEffect
