#include "libraries_panel.h"
#include "../mainwindow/main_window.h"
#include <QGridLayout>
#include <QFrame>
#include <QFileDialog>
#include <QColorDialog>

namespace FreeEffect {

LibrariesPanel::LibrariesPanel(MainWindow* parent)
    : QWidget(parent)
    , m_mainWindow(parent) {
    setupUi();
}

void LibrariesPanel::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(6);

    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("Search libraries...");
    mainLayout->addWidget(m_searchEdit);

    m_categoryCombo = new QComboBox(this);
    m_categoryCombo->addItem("All Assets");
    m_categoryCombo->addItem("Graphics");
    m_categoryCombo->addItem("Color Swatches");
    m_categoryCombo->addItem("Character Styles");
    m_categoryCombo->addItem("Graphic Styles");
    mainLayout->addWidget(m_categoryCombo);

    QGroupBox* assetsGroup = new QGroupBox("Assets", this);
    QVBoxLayout* aLayout = new QVBoxLayout(assetsGroup);

    m_assetsList = new QListWidget(this);
    m_assetsList->setIconSize(QSize(48, 48));
    m_assetsList->setResizeMode(QListWidget::Adjust);
    m_assetsList->setViewMode(QListWidget::IconMode);
    m_assetsList->setMovement(QListWidget::Static);
    aLayout->addWidget(m_assetsList);

    connect(m_assetsList, &QListWidget::itemDoubleClicked, this, &LibrariesPanel::onAssetDoubleClicked);

    QHBoxLayout* abLayout = new QHBoxLayout();
    m_importBtn = new QPushButton("Import", this);
    m_deleteBtn = new QPushButton("Delete", this);
    m_shareBtn = new QPushButton("Share", this);
    abLayout->addWidget(m_importBtn);
    abLayout->addWidget(m_deleteBtn);
    abLayout->addWidget(m_shareBtn);
    aLayout->addLayout(abLayout);

    connect(m_importBtn, &QPushButton::clicked, this, &LibrariesPanel::onImportAsset);
    connect(m_deleteBtn, &QPushButton::clicked, this, &LibrariesPanel::onDeleteAsset);
    connect(m_shareBtn, &QPushButton::clicked, this, &LibrariesPanel::onShareAsset);

    mainLayout->addWidget(assetsGroup);

    QFrame* sep1 = new QFrame(this);
    sep1->setFrameShape(QFrame::HLine);
    sep1->setFrameShadow(QFrame::Sunken);
    mainLayout->addWidget(sep1);

    QGroupBox* swatchGroup = new QGroupBox("Color Swatches", this);
    QVBoxLayout* swLayout = new QVBoxLayout(swatchGroup);

    m_swatchesList = new QListWidget(this);
    m_swatchesList->setMaximumHeight(80);
    swLayout->addWidget(m_swatchesList);

    m_addSwatchBtn = new QPushButton("Add Swatch", this);
    swLayout->addWidget(m_addSwatchBtn);
    connect(m_addSwatchBtn, &QPushButton::clicked, this, &LibrariesPanel::onAddColorSwatch);

    mainLayout->addWidget(swatchGroup);

    QGroupBox* styleGroup = new QGroupBox("Graphic Styles", this);
    QVBoxLayout* stLayout = new QVBoxLayout(styleGroup);

    m_stylesList = new QListWidget(this);
    m_stylesList->setMaximumHeight(80);
    stLayout->addWidget(m_stylesList);

    m_addStyleBtn = new QPushButton("Add Style", this);
    stLayout->addWidget(m_addStyleBtn);
    connect(m_addStyleBtn, &QPushButton::clicked, this, &LibrariesPanel::onAddGraphicStyle);

    mainLayout->addWidget(styleGroup);
    mainLayout->addStretch();
}

void LibrariesPanel::onImportAsset() {
    QString file = QFileDialog::getOpenFileName(this, "Import Asset", QString(),
        "All Files (*)");
    if (!file.isEmpty()) {
        QListWidgetItem* item = new QListWidgetItem(
            QIcon(":/icons/file.svg"),
            file.section("/", -1),
            m_assetsList);
        item->setData(Qt::UserRole, file);
        emit importAssetRequested();
    }
}

void LibrariesPanel::onDeleteAsset() {
    QListWidgetItem* item = m_assetsList->currentItem();
    if (item) {
        delete item;
        emit deleteAssetRequested();
    }
}

void LibrariesPanel::onShareAsset() { emit shareAssetRequested(); }

void LibrariesPanel::onAddColorSwatch() {
    QColor color = QColorDialog::getColor(Qt::white, this, "Select Color");
    if (color.isValid()) {
        QPixmap pix(24, 24);
        pix.fill(color);
        QListWidgetItem* item = new QListWidgetItem(QIcon(pix), color.name(), m_swatchesList);
        item->setData(Qt::UserRole, color);
        emit colorSwatchSelected(color);
    }
}

void LibrariesPanel::onAddGraphicStyle() {
    QListWidgetItem* item = new QListWidgetItem("New Style", m_stylesList);
    emit graphicStyleApplied("New Style");
}

void LibrariesPanel::onAssetDoubleClicked(QListWidgetItem* item) {
    if (item) {
        QString path = item->data(Qt::UserRole).toString();
    }
}

} // namespace FreeEffect
