#include "project_panel.h"
#include "../mainwindow/main_window.h"
#include <QHeaderView>
#include <QFileDialog>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include "../../core/io/importer.h"

namespace FreeEffect {

ProjectPanel::ProjectPanel(MainWindow* parent)
    : QWidget(parent)
    , m_mainWindow(parent) {
    setupUi();
    setAcceptDrops(true);
    refreshAssetList();
}

void ProjectPanel::setupUi() {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    
    // Search bar
    QWidget* searchWidget = new QWidget(this);
    searchWidget->setFixedHeight(28);
    searchWidget->setStyleSheet("background-color: #2d2d2d; border-bottom: 1px solid #1a1a1a;");
    QHBoxLayout* searchLayout = new QHBoxLayout(searchWidget);
    searchLayout->setContentsMargins(6, 4, 6, 4);
    searchLayout->setSpacing(4);
    
    m_searchEdit = new QLineEdit(searchWidget);
    m_searchEdit->setPlaceholderText("Search...");
    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setStyleSheet(
        "QLineEdit { background-color: #3c3c3c; color: #cccccc; border: 1px solid #555555; "
        "border-radius: 3px; padding: 2px 6px; font-size: 11px; }"
        "QLineEdit:focus { border: 1px solid #2680eb; }"
    );
    connect(m_searchEdit, &QLineEdit::textChanged, this, &ProjectPanel::onSearchChanged);
    searchLayout->addWidget(m_searchEdit);
    
    layout->addWidget(searchWidget);
    
    // Tree widget
    m_treeWidget = new QTreeWidget(this);
    m_treeWidget->setObjectName("ProjectTree");
    m_treeWidget->setHeaderLabels(QStringList() << "Name" << "Type" << "Size" << "Duration" << "Frame Rate");
    m_treeWidget->setColumnWidth(0, 180);
    m_treeWidget->setColumnWidth(1, 60);
    m_treeWidget->setColumnWidth(2, 60);
    m_treeWidget->setColumnWidth(3, 60);
    m_treeWidget->setColumnWidth(4, 60);
    m_treeWidget->setRootIsDecorated(true);
    m_treeWidget->setAlternatingRowColors(false);
    m_treeWidget->header()->setStretchLastSection(false);
    m_treeWidget->header()->setSectionResizeMode(0, QHeaderView::Interactive);
    m_treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    m_treeWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    m_treeWidget->setIndentation(16);
    m_treeWidget->setStyleSheet(
        "QTreeWidget { background-color: #2a2a2a; color: #cccccc; border: none; font-size: 11px; }"
        "QTreeWidget::item { padding: 2px 4px; border: none; height: 22px; }"
        "QTreeWidget::item:selected { background-color: #2680eb; color: white; }"
        "QTreeWidget::item:hover { background-color: #353535; }"
        "QHeaderView::section { background-color: #2d2d2d; color: #888888; border: none; "
        "border-right: 1px solid #1a1a1a; padding: 3px 4px; font-size: 10px; }"
    );
    
    connect(m_treeWidget, &QTreeWidget::customContextMenuRequested, this, &ProjectPanel::onContextMenu);
    connect(m_treeWidget, &QTreeWidget::itemDoubleClicked, this, &ProjectPanel::onItemDoubleClicked);
    
    layout->addWidget(m_treeWidget, 1);
}

void ProjectPanel::refreshAssetList() {
    m_treeWidget->clear();
    
    if (!m_mainWindow) return;
    
    auto& project = m_mainWindow->getProjectState();
    
    // Folders first
    QTreeWidgetItem* folderItem = new QTreeWidgetItem(m_treeWidget);
    folderItem->setText(0, "Assets");
    folderItem->setIcon(0, QIcon(":/icons/folder.svg"));
    folderItem->setExpanded(true);
    QFont boldFont = folderItem->font(0);
    boldFont.setBold(true);
    folderItem->setFont(0, boldFont);
    folderItem->setForeground(0, QColor(180, 180, 180));
    
    for (const auto& asset : project.getAssets()) {
        QTreeWidgetItem* item = new QTreeWidgetItem(folderItem);
        item->setText(0, QString::fromStdString(asset->getName()));
        
        QString typeStr;
        QIcon icon;
        switch (asset->getType()) {
            case AssetType::Video:
                typeStr = "Video";
                icon = QIcon(":/icons/layer/video.svg");
                break;
            case AssetType::Image:
                typeStr = "Image";
                icon = QIcon(":/icons/layer/image.svg");
                break;
            case AssetType::Audio:
                typeStr = "Audio";
                icon = QIcon(":/icons/layer/audio.svg");
                break;
            case AssetType::Composition:
                typeStr = "Comp";
                icon = QIcon(":/icons/panels/composition.svg");
                break;
        }
        item->setText(1, typeStr);
        item->setIcon(0, icon);
        item->setData(0, Qt::UserRole, QString::fromStdString(asset->getId()));
        
        if (asset->getStatus() == AssetStatus::Missing) {
            item->setIcon(0, QIcon(":/icons/status/missing.svg"));
            item->setForeground(0, QColor(255, 80, 80));
        }
    }
    
    // Compositions folder
    if (!project.getCompositions().empty()) {
        QTreeWidgetItem* compFolder = new QTreeWidgetItem(m_treeWidget);
        compFolder->setText(0, "Compositions");
        compFolder->setIcon(0, QIcon(":/icons/folder.svg"));
        compFolder->setExpanded(true);
        compFolder->setFont(0, boldFont);
        compFolder->setForeground(0, QColor(180, 180, 180));
        
        for (const auto& comp : project.getCompositions()) {
            QTreeWidgetItem* item = new QTreeWidgetItem(compFolder);
            item->setText(0, QString::fromStdString(comp->getName()));
            item->setText(1, "Composition");
            item->setIcon(0, QIcon(":/icons/panels/composition.svg"));
            item->setData(0, Qt::UserRole, QString::fromStdString(comp->getId()));
        }
    }
}

void ProjectPanel::setSelectedAsset(const UUID& assetId) {
    m_selectedAssetId = assetId;
}

void ProjectPanel::onContextMenu(const QPoint& pos) {
    QMenu menu(this);
    menu.setStyleSheet(
        "QMenu { background-color: #3c3c3c; color: #cccccc; border: 1px solid #555555; }"
        "QMenu::item { padding: 5px 25px; }"
        "QMenu::item:selected { background-color: #2680eb; color: white; }"
        "QMenu::separator { height: 1px; background: #555555; margin: 4px 10px; }"
    );
    
    QAction* importAction = menu.addAction("Import File...");
    connect(importAction, &QAction::triggered, m_mainWindow, &MainWindow::onImportFile);
    
    menu.addSeparator();
    
    QAction* newCompAction = menu.addAction("New Composition...");
    connect(newCompAction, &QAction::triggered, m_mainWindow, &MainWindow::onNewComposition);
    
    menu.addAction("New Folder");
    menu.addSeparator();
    
    QTreeWidgetItem* item = m_treeWidget->itemAt(pos);
    if (item) {
        menu.addAction("Replace Footage...");
        menu.addAction("Reveal in Finder");
        menu.addSeparator();
        menu.addAction("Delete");
    }
    
    menu.exec(m_treeWidget->mapToGlobal(pos));
}

void ProjectPanel::onItemDoubleClicked(QTreeWidgetItem* item, int column) {
    Q_UNUSED(column);
    if (!item) return;
    
    UUID id = item->data(0, Qt::UserRole).toString().toStdString();
    if (!id.empty()) {
        emit assetDoubleClicked(id);
    }
}

void ProjectPanel::onSearchChanged(const QString& text) {
    for (int i = 0; i < m_treeWidget->topLevelItemCount(); ++i) {
        QTreeWidgetItem* folder = m_treeWidget->topLevelItem(i);
        for (int j = 0; j < folder->childCount(); ++j) {
            QTreeWidgetItem* child = folder->child(j);
            if (text.isEmpty() || child->text(0).contains(text, Qt::CaseInsensitive)) {
                child->setHidden(false);
            } else {
                child->setHidden(true);
            }
        }
    }
}

void ProjectPanel::dragEnterEvent(QDragEnterEvent* event) {
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void ProjectPanel::dropEvent(QDropEvent* event) {
    const QMimeData* mime = event->mimeData();
    if (!mime->hasUrls()) return;
    
    for (const QUrl& url : mime->urls()) {
        if (url.isLocalFile()) {
            QString filePath = url.toLocalFile();
            Importer importer(m_mainWindow->getProjectStatePtr());
            auto asset = importer.importFile(filePath.toStdString());
            if (asset) {
                m_mainWindow->getProjectState().setModified(true);
                m_mainWindow->updateTitle();
                refreshAssetList();
            }
        }
    }
}

} // namespace FreeEffect
