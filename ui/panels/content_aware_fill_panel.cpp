#include "content_aware_fill_panel.h"
#include "../mainwindow/main_window.h"
#include <QGridLayout>
#include <QFrame>
#include <QCheckBox>

namespace FreeEffect {

ContentAwareFillPanel::ContentAwareFillPanel(MainWindow* parent)
    : QWidget(parent)
    , m_mainWindow(parent) {
    setupUi();
}

void ContentAwareFillPanel::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(6);

    QGroupBox* methodGroup = new QGroupBox("Fill Settings", this);
    QGridLayout* mLayout = new QGridLayout(methodGroup);

    mLayout->addWidget(new QLabel("Method:", this), 0, 0);
    m_methodCombo = new QComboBox(this);
    m_methodCombo->addItem("Object");
    m_methodCombo->addItem("Surface");
    m_methodCombo->addItem("Edge Blend");
    m_methodCombo->addItem("Fill Light");
    mLayout->addWidget(m_methodCombo, 0, 1);

    mLayout->addWidget(new QLabel("Range:", this), 1, 0);
    m_rangeCombo = new QComboBox(this);
    m_rangeCombo->addItem("Work Area");
    m_rangeCombo->addItem("Entire Duration");
    mLayout->addWidget(m_rangeCombo, 1, 1);

    mLayout->addWidget(new QLabel("Fill Target:", this), 2, 0);
    m_fillTargetCombo = new QComboBox(this);
    m_fillTargetCombo->addItem("New Solid");
    m_fillTargetCombo->addItem("Existing Layer");
    mLayout->addWidget(m_fillTargetCombo, 2, 1);

    m_fillEmptyCheck = new QCheckBox("Fill Empty Only", this);
    m_fillEmptyCheck->setChecked(true);
    mLayout->addWidget(m_fillEmptyCheck, 3, 0, 1, 2);

    connect(m_methodCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ContentAwareFillPanel::onMethodChanged);

    mainLayout->addWidget(methodGroup);

    QFrame* sep = new QFrame(this);
    sep->setFrameShape(QFrame::HLine);
    sep->setFrameShadow(QFrame::Sunken);
    mainLayout->addWidget(sep);

    QGroupBox* actionGroup = new QGroupBox("Actions", this);
    QVBoxLayout* aLayout = new QVBoxLayout(actionGroup);

    m_createRefFrameBtn = new QPushButton("Create Reference Frame", this);
    m_generateFillBtn = new QPushButton("Generate Fill Layer", this);
    m_cancelBtn = new QPushButton("Cancel", this);

    aLayout->addWidget(m_createRefFrameBtn);
    aLayout->addWidget(m_generateFillBtn);
    aLayout->addWidget(m_cancelBtn);

    connect(m_createRefFrameBtn, &QPushButton::clicked, this, &ContentAwareFillPanel::onCreateReferenceFrame);
    connect(m_generateFillBtn, &QPushButton::clicked, this, &ContentAwareFillPanel::onGenerateFill);
    connect(m_cancelBtn, &QPushButton::clicked, this, &ContentAwareFillPanel::onCancel);

    mainLayout->addWidget(actionGroup);

    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setVisible(false);
    mainLayout->addWidget(m_progressBar);

    m_statusLabel = new QLabel("Ready", this);
    m_statusLabel->setStyleSheet("color: #888888; font-size: 11px;");
    mainLayout->addWidget(m_statusLabel);

    mainLayout->addStretch();
}

void ContentAwareFillPanel::onCreateReferenceFrame() {
    m_statusLabel->setText("Creating reference frame...");
    emit createReferenceFrameRequested();
}

void ContentAwareFillPanel::onGenerateFill() {
    m_progressBar->setVisible(true);
    m_progressBar->setValue(0);
    m_statusLabel->setText("Generating fill layer...");
    emit generateFillLayerRequested();
}

void ContentAwareFillPanel::onCancel() {
    m_progressBar->setVisible(false);
    m_statusLabel->setText("Cancelled");
    emit cancelRequested();
}

void ContentAwareFillPanel::onMethodChanged(int index) {}

} // namespace FreeEffect
