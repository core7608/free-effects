#pragma once

#include <QDockWidget>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QMenu>
#include <QAction>
#include "../../core/project/project_state.h"

namespace FreeEffect {

class MainWindow;

class ProjectPanel : public QWidget {
    Q_OBJECT
public:
    explicit ProjectPanel(MainWindow* parent);
    ~ProjectPanel() override = default;
    
    void refreshAssetList();
    void setSelectedAsset(const UUID& assetId);

signals:
    void assetSelected(const UUID& assetId);
    void assetDoubleClicked(const UUID& assetId);

private slots:
    void onContextMenu(const QPoint& pos);
    void onItemDoubleClicked(QTreeWidgetItem* item, int column);

private:
    void setupUi();
    void populateTree();
    
    MainWindow* m_mainWindow;
    QTreeWidget* m_treeWidget;
    UUID m_selectedAssetId;
};

} // namespace FreeEffect
