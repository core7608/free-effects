#pragma once

#include <QMenuBar>
#include <QAction>
#include <QMenu>
#include "shortcut_manager.h"

namespace FreeEffect {

class MainWindow;

class MenuBar : public QMenuBar {
    Q_OBJECT
public:
    explicit MenuBar(MainWindow* parent);
    ~MenuBar() override = default;

private:
    void createFileMenu();
    void createEditMenu();
    void createCompositionMenu();
    void createLayerMenu();
    void createEffectMenu();
    void createAnimationMenu();
    void createViewMenu();
    void createWindowMenu();
    void createHelpMenu();
    
    QAction* createNamedAction(const QString& name, const QString& text, QMenu* menu = nullptr);
    QAction* createNamedActionWithShortcut(const QString& name, const QString& text, 
                                           ShortcutAction shortcut, QMenu* menu = nullptr);
    QAction* createMenuAction(const QString& text, QMenu* menu = nullptr);
    QAction* createMenuActionWithShortcut(const QString& text, ShortcutAction shortcut, QMenu* menu = nullptr);
    
    MainWindow* m_mainWindow;
    
    QMenu* m_fileMenu;
    QMenu* m_editMenu;
    QMenu* m_compositionMenu;
    QMenu* m_layerMenu;
    QMenu* m_effectMenu;
    QMenu* m_animationMenu;
    QMenu* m_viewMenu;
    QMenu* m_windowMenu;
    QMenu* m_helpMenu;
};

} // namespace FreeEffect
