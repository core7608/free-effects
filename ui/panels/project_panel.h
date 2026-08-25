#pragma once

#include <QDockWidget>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QLineEdit>
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
    
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;

signals:
    void assetSelected(const UUID& assetId);
    void assetDoubleClicked(const UUID& assetId);

private slots:
    void onContextMenu(const QPoint& pos);
    void onItemDoubleClicked(QTreeWidgetItem* item, int column);
    void onSearchChanged(const QString& text);

private:
    void setupUi();
    void populateTree();
    
    MainWindow* m_mainWindow;
    QTreeWidget* m_treeWidget;
    QLineEdit* m_searchEdit;
    UUID m_selectedAssetId;
};

} // namespace FreeEffect
