#include "roto_brush_panel.h"
#include "../mainwindow/main_window.h"
#include <QGridLayout>
#include <QFrame>
#include <QGroupBox>
#include <QButtonGroup>

namespace FreeEffect {

RotoBrushPanel::RotoBrushPanel(MainWindow* parent)
    : QWidget(parent)
    , m_mainWindow(parent) {
    setupUi();
}

void RotoBrushPanel::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(6);

    QGroupBox* brushGroup = new QGroupBox("Brush Settings", this);
    QGridLayout* bLayout = new QGridLayout(brushGroup);

    bLayout->addWidget(new QLabel("Size:", this), 0, 0);
    m_brushSizeSlider = new QSlider(Qt::Horizontal, this);
    m_brushSizeSlider->setRange(1, 200);
    m_brushSizeSlider->setValue(25);
    m_brushSizeLabel = new QLabel("25", this);
    bLayout->addWidget(m_brushSizeSlider, 0, 1);
    bLayout->addWidget(m_brushSizeLabel, 0, 2);
    connect(m_brushSizeSlider, &QSlider::valueChanged, this, &RotoBrushPanel::onBrushSizeChanged);

    bLayout->addWidget(new QLabel("Feather:", this), 1, 0);
    m_featherSlider = new QSlider(Qt::Horizontal, this);
    m_featherSlider->setRange(0, 100);
    m_featherSlider->setValue(10);
    m_featherLabel = new QLabel("10", this);
    bLayout->addWidget(m_featherSlider, 1, 1);
    bLayout->addWidget(m_featherLabel, 1, 2);
    connect(m_featherSlider, &QSlider::valueChanged, this, &RotoBrushPanel::onFeatherChanged);

    bLayout->addWidget(new QLabel("Contrast:", this), 2, 0);
    m_contrastSlider = new QSlider(Qt::Horizontal, this);
    m_contrastSlider->setRange(0, 100);
    m_contrastSlider->setValue(50);
    m_contrastLabel = new QLabel("50", this);
    bLayout->addWidget(m_contrastSlider, 2, 1);
    bLayout->addWidget(m_contrastLabel, 2, 2);
    connect(m_contrastSlider, &QSlider::valueChanged, this, &RotoBrushPanel::onContrastChanged);

    mainLayout->addWidget(brushGroup);

    QGroupBox* modeGroup = new QGroupBox("Paint Mode", this);
    QVBoxLayout* mLayout = new QVBoxLayout(modeGroup);

    QButtonGroup* modeGroup2 = new QButtonGroup(this);
    m_foregroundRadio = new QRadioButton("Foreground (Add to selection)", this);
    m_foregroundRadio->setChecked(true);
    m_backgroundRadio = new QRadioButton("Background (Subtract from selection)", this);
    modeGroup2->addButton(m_foregroundRadio);
    modeGroup2->addButton(m_backgroundRadio);
    mLayout->addWidget(m_foregroundRadio);
    mLayout->addWidget(m_backgroundRadio);

    connect(m_foregroundRadio, &QRadioButton::toggled, this, [this](bool checked) {
        if (checked) emit toggleForegroundMode();
    });
    connect(m_backgroundRadio, &QRadioButton::toggled, this, [this](bool checked) {
        if (checked) emit toggleBackgroundMode();
    });

    mainLayout->addWidget(modeGroup);

    QGroupBox* edgeGroup = new QGroupBox("Edge Detection", this);
    QVBoxLayout* eLayout = new QVBoxLayout(edgeGroup);
    m_edgeDetectionCombo = new QComboBox(this);
    m_edgeDetectionCombo->addItem("Canny");
    m_edgeDetectionCombo->addItem("Sobel");
    m_edgeDetectionCombo->addItem("Morphological");
    eLayout->addWidget(m_edgeDetectionCombo);
    mainLayout->addWidget(edgeGroup);

    QFrame* sep = new QFrame(this);
    sep->setFrameShape(QFrame::HLine);
    sep->setFrameShadow(QFrame::Sunken);
    mainLayout->addWidget(sep);

    QGroupBox* propagateGroup = new QGroupBox("Propagation", this);
    QVBoxLayout* pLayout = new QVBoxLayout(propagateGroup);

    m_propagateForwardBtn = new QPushButton("Propagate Forward", this);
    m_propagateBackwardBtn = new QPushButton("Propagate Backward", this);
    m_refineEdgeBtn = new QPushButton("Refine Edge", this);
    m_freezeBtn = new QPushButton("Freeze", this);
    m_unfreezeBtn = new QPushButton("Unfreeze", this);

    pLayout->addWidget(m_propagateForwardBtn);
    pLayout->addWidget(m_propagateBackwardBtn);
    pLayout->addWidget(m_refineEdgeBtn);
    pLayout->addWidget(m_freezeBtn);
    pLayout->addWidget(m_unfreezeBtn);

    connect(m_propagateForwardBtn, &QPushButton::clicked, this, &RotoBrushPanel::onPropagateForward);
    connect(m_propagateBackwardBtn, &QPushButton::clicked, this, &RotoBrushPanel::onPropagateBackward);
    connect(m_refineEdgeBtn, &QPushButton::clicked, this, &RotoBrushPanel::onRefineEdge);
    connect(m_freezeBtn, &QPushButton::clicked, this, &RotoBrushPanel::onFreeze);
    connect(m_unfreezeBtn, &QPushButton::clicked, this, &RotoBrushPanel::onUnfreeze);

    mainLayout->addWidget(propagateGroup);

    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setVisible(false);
    mainLayout->addWidget(m_progressBar);

    mainLayout->addStretch();
}

void RotoBrushPanel::onBrushSizeChanged(int value) { m_brushSizeLabel->setText(QString::number(value)); }
void RotoBrushPanel::onFeatherChanged(int value) { m_featherLabel->setText(QString::number(value)); }
void RotoBrushPanel::onContrastChanged(int value) { m_contrastLabel->setText(QString::number(value)); }
void RotoBrushPanel::onPropagateForward() {
    m_progressBar->setVisible(true);
    m_progressBar->setValue(0);
    emit propagateForwardRequested();
}
void RotoBrushPanel::onPropagateBackward() {
    m_progressBar->setVisible(true);
    m_progressBar->setValue(0);
    emit propagateBackwardRequested();
}
void RotoBrushPanel::onFreeze() { emit freezeLayerRequested(); }
void RotoBrushPanel::onUnfreeze() { emit unfreezeLayerRequested(); }
void RotoBrushPanel::onRefineEdge() { emit refineEdgeRequested(); }

} // namespace FreeEffect
