#include "project_panel.h"
#include "../mainwindow/main_window.h"
#include <QHeaderView>
#include <QFileDialog>

namespace FreeEffect {

ProjectPanel::ProjectPanel(MainWindow* parent)
    : QWidget(parent)
    , m_mainWindow(parent) {
    setupUi();
    refreshAssetList();
}

void ProjectPanel::setupUi() {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    
    m_treeWidget = new QTreeWidget(this);
    m_treeWidget->setHeaderLabels({"Name", "Type", "Size", "Duration", "Frame Rate"});
    m_treeWidget->setColumnWidth(0, 180);
    m_treeWidget->setColumnWidth(1, 60);
    m_treeWidget->setColumnWidth(2, 60);
    m_treeWidget->setColumnWidth(3, 60);
    m_treeWidget->setColumnWidth(4, 60);
    m_treeWidget->setRootIsDecorated(true);
    m_treeWidget->setAlternatingRowColors(true);
    m_treeWidget->header()->setStretchLastSection(false);
    m_treeWidget->header()->setSectionResizeMode(0, QHeaderView::Interactive);
    m_treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    m_treeWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    
    connect(m_treeWidget, &QTreeWidget::customContextMenuRequested, this, &ProjectPanel::onContextMenu);
    connect(m_treeWidget, &QTreeWidget::itemDoubleClicked, this, &ProjectPanel::onItemDoubleClicked);
    
    layout->addWidget(m_treeWidget);
}

void ProjectPanel::refreshAssetList() {
    m_treeWidget->clear();
    
    if (!m_mainWindow) return;
    
    auto& project = m_mainWindow->getProjectState();
    
    for (const auto& asset : project.getAssets()) {
        QTreeWidgetItem* item = new QTreeWidgetItem(m_treeWidget);
        item->setText(0, QString::fromStdString(asset->getName()));
        
        QString typeStr;
        switch (asset->getType()) {
            case AssetType::Video: typeStr = "Video"; break;
            case AssetType::Image: typeStr = "Image"; break;
            case AssetType::Audio: typeStr = "Audio"; break;
            case AssetType::Composition: typeStr = "Comp"; break;
        }
        item->setText(1, typeStr);
        item->setData(0, Qt::UserRole, QString::fromStdString(asset->getId()));
        
        if (asset->getStatus() == AssetStatus::Missing) {
            item->setIcon(0, QIcon(":/icons/missing_footage.svg"));
            item->setForeground(0, Qt::red);
        } else {
            if (asset->getType() == AssetType::Video) item->setIcon(0, QIcon(":/icons/video_file.svg"));
            else if (asset->getType() == AssetType::Image) item->setIcon(0, QIcon(":/icons/image_file.svg"));
            else if (asset->getType() == AssetType::Audio) item->setIcon(0, QIcon(":/icons/audio_file.svg"));
        }
    }
    
    for (const auto& comp : project.getCompositions()) {
        QTreeWidgetItem* item = new QTreeWidgetItem(m_treeWidget);
        item->setText(0, QString::fromStdString(comp->getName()));
        item->setText(1, "Composition");
        item->setIcon(0, QIcon(":/icons/composition.svg"));
        item->setData(0, Qt::UserRole, QString::fromStdString(comp->getId()));
    }
}

void ProjectPanel::setSelectedAsset(const UUID& assetId) {
    m_selectedAssetId = assetId;
}

void ProjectPanel::onContextMenu(const QPoint& pos) {
    QMenu menu(this);
    menu.addAction("Import File...");
    menu.addSeparator();
    menu.addAction("New Composition...");
    menu.addAction("New Folder");
    menu.addSeparator();
    
    QTreeWidgetItem* item = m_treeWidget->itemAt(pos);
    if (item) {
        menu.addAction("Replace Footage...");
        menu.addAction("Delete");
    }
    
    menu.exec(m_treeWidget->mapToGlobal(pos));
}

void ProjectPanel::onItemDoubleClicked(QTreeWidgetItem* item, int column) {
    Q_UNUSED(column);
    if (!item) return;
    
    UUID id = item->data(0, Qt::UserRole).toString().toStdString();
    emit assetDoubleClicked(id);
}

} // namespace FreeEffect
