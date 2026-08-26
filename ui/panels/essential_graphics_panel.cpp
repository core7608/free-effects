#include "essential_graphics_panel.h"
#include "../mainwindow/main_window.h"
#include <QGridLayout>
#include <QFrame>
#include <QFileDialog>

namespace FreeEffect {

EssentialGraphicsPanel::EssentialGraphicsPanel(MainWindow* parent)
    : QWidget(parent)
    , m_mainWindow(parent) {
    setupUi();
}

void EssentialGraphicsPanel::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(6);

    QGroupBox* templateGroup = new QGroupBox("Template Settings", this);
    QGridLayout* tLayout = new QGridLayout(templateGroup);

    tLayout->addWidget(new QLabel("Name:", this), 0, 0);
    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setPlaceholderText("Template name");
    tLayout->addWidget(m_nameEdit, 0, 1);

    tLayout->addWidget(new QLabel("Description:", this), 1, 0);
    m_descriptionEdit = new QTextEdit(this);
    m_descriptionEdit->setMaximumHeight(60);
    m_descriptionEdit->setPlaceholderText("Template description");
    tLayout->addWidget(m_descriptionEdit, 1, 1);

    mainLayout->addWidget(templateGroup);

    QGroupBox* propsGroup = new QGroupBox("Exposed Properties", this);
    QVBoxLayout* pLayout = new QVBoxLayout(propsGroup);

    m_propertiesTree = new QTreeWidget(this);
    m_propertiesTree->setHeaderLabels({"Property", "Type", "Range"});
    m_propertiesTree->setColumnWidth(0, 150);
    m_propertiesTree->setColumnWidth(1, 80);
    m_propertiesTree->setColumnWidth(2, 100);
    pLayout->addWidget(m_propertiesTree);

    connect(m_propertiesTree, &QTreeWidget::itemClicked, this, &EssentialGraphicsPanel::onPropertySelected);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    m_addPropertyBtn = new QPushButton("Add", this);
    m_removePropertyBtn = new QPushButton("Remove", this);
    btnLayout->addWidget(m_addPropertyBtn);
    btnLayout->addWidget(m_removePropertyBtn);
    pLayout->addLayout(btnLayout);

    connect(m_addPropertyBtn, &QPushButton::clicked, this, &EssentialGraphicsPanel::onAddProperty);
    connect(m_removePropertyBtn, &QPushButton::clicked, this, &EssentialGraphicsPanel::onRemoveProperty);

    mainLayout->addWidget(propsGroup);

    QGroupBox* rangeGroup = new QGroupBox("Property Range", this);
    QGridLayout* rLayout = new QGridLayout(rangeGroup);

    rLayout->addWidget(new QLabel("Min:", this), 0, 0);
    m_rangeMinSpin = new QDoubleSpinBox(this);
    m_rangeMinSpin->setRange(-99999, 99999);
    rLayout->addWidget(m_rangeMinSpin, 0, 1);

    rLayout->addWidget(new QLabel("Max:", this), 1, 0);
    m_rangeMaxSpin = new QDoubleSpinBox(this);
    m_rangeMaxSpin->setRange(-99999, 99999);
    m_rangeMaxSpin->setValue(100);
    rLayout->addWidget(m_rangeMaxSpin, 1, 1);

    mainLayout->addWidget(rangeGroup);

    QFrame* sep = new QFrame(this);
    sep->setFrameShape(QFrame::HLine);
    sep->setFrameShadow(QFrame::Sunken);
    mainLayout->addWidget(sep);

    QGroupBox* actionGroup = new QGroupBox("Actions", this);
    QVBoxLayout* aLayout = new QVBoxLayout(actionGroup);

    m_previewBtn = new QPushButton("Preview Template", this);
    m_exportMOGRTBtn = new QPushButton("Export MOGRT", this);

    aLayout->addWidget(m_previewBtn);
    aLayout->addWidget(m_exportMOGRTBtn);

    connect(m_previewBtn, &QPushButton::clicked, this, &EssentialGraphicsPanel::onPreviewTemplate);
    connect(m_exportMOGRTBtn, &QPushButton::clicked, this, &EssentialGraphicsPanel::onExportMOGRT);

    mainLayout->addWidget(actionGroup);

    m_statusLabel = new QLabel("Ready", this);
    m_statusLabel->setStyleSheet("color: #888888; font-size: 11px;");
    mainLayout->addWidget(m_statusLabel);

    mainLayout->addStretch();
}

void EssentialGraphicsPanel::onAddProperty() {
    QTreeWidgetItem* item = new QTreeWidgetItem(m_propertiesTree);
    item->setText(0, "New Property");
    item->setText(1, "float");
    item->setText(2, "0 - 100");
    emit addPropertyRequested();
}

void EssentialGraphicsPanel::onRemoveProperty() {
    QTreeWidgetItem* item = m_propertiesTree->currentItem();
    if (item) {
        delete item;
        emit removePropertyRequested();
    }
}

void EssentialGraphicsPanel::onExportMOGRT() {
    QString file = QFileDialog::getSaveFileName(this, "Export MOGRT", QString(),
        "Motion Graphics Templates (*.mogrt);;All Files (*)");
    if (!file.isEmpty()) {
        m_statusLabel->setText("Exporting...");
        emit exportMOGRTRequested();
    }
}

void EssentialGraphicsPanel::onPreviewTemplate() { emit previewTemplateRequested(); }

void EssentialGraphicsPanel::onPropertySelected(QTreeWidgetItem* item, int column) {
    if (item) {
        m_statusLabel->setText("Selected: " + item->text(0));
    }
}

} // namespace FreeEffect
