#include "tracker_panel.h"
#include "../mainwindow/main_window.h"
#include <QGridLayout>
#include <QFrame>
#include <QGroupBox>

namespace FreeEffect {

TrackerPanel::TrackerPanel(MainWindow* parent)
    : QWidget(parent)
    , m_mainWindow(parent) {
    setupUi();
}

void TrackerPanel::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(6);

    QGroupBox* trackTypeGroup = new QGroupBox("Track Type", this);
    QVBoxLayout* ttLayout = new QVBoxLayout(trackTypeGroup);

    m_trackTypeCombo = new QComboBox(this);
    m_trackTypeCombo->addItem("Track Motion");
    m_trackTypeCombo->addItem("Stabilize Motion");
    m_trackTypeCombo->addItem("3D Camera Tracker");
    m_trackTypeCombo->addItem("Track in Mocha");
    ttLayout->addWidget(m_trackTypeCombo);

    m_trackForCombo = new QComboBox(this);
    m_trackForCombo->addItem("Position");
    m_trackForCombo->addItem("Position & Rotation");
    m_trackForCombo->addItem("Position, Scale & Rotation");
    m_trackForCombo->addItem("Perspective");
    ttLayout->addWidget(m_trackForCombo);

    connect(m_trackTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &TrackerPanel::onTrackTypeChanged);

    mainLayout->addWidget(trackTypeGroup);

    QGroupBox* trackOptionsGroup = new QGroupBox("Track Options", this);
    QGridLayout* toLayout = new QGridLayout(trackOptionsGroup);

    toLayout->addWidget(new QLabel("Search Region:", this), 0, 0);
    m_searchRegionSpin = new QSpinBox(this);
    m_searchRegionSpin->setRange(5, 500);
    m_searchRegionSpin->setValue(50);
    toLayout->addWidget(m_searchRegionSpin, 0, 1);

    toLayout->addWidget(new QLabel("Feature Region:", this), 1, 0);
    m_featureRegionSpin = new QSpinBox(this);
    m_featureRegionSpin->setRange(3, 200);
    m_featureRegionSpin->setValue(20);
    toLayout->addWidget(m_featureRegionSpin, 1, 1);

    toLayout->addWidget(new QLabel("Confidence:", this), 2, 0);
    m_confidenceThresholdSpin = new QDoubleSpinBox(this);
    m_confidenceThresholdSpin->setRange(0.0, 1.0);
    m_confidenceThresholdSpin->setValue(0.3);
    m_confidenceThresholdSpin->setSingleStep(0.05);
    toLayout->addWidget(m_confidenceThresholdSpin, 2, 1);

    toLayout->addWidget(new QLabel("Frames Back:", this), 3, 0);
    m_framesBackSpin = new QSpinBox(this);
    m_framesBackSpin->setRange(1, 1000);
    m_framesBackSpin->setValue(10);
    toLayout->addWidget(m_framesBackSpin, 3, 1);

    m_adaptiveFeatureCheck = new QCheckBox("Adaptive Feature", this);
    m_adaptiveFeatureCheck->setChecked(true);
    toLayout->addWidget(m_adaptiveFeatureCheck, 4, 0, 1, 2);

    m_subpixelCheck = new QCheckBox("Sub-pixel Tracking", this);
    m_subpixelCheck->setChecked(true);
    toLayout->addWidget(m_subpixelCheck, 5, 0, 1, 2);

    mainLayout->addWidget(trackOptionsGroup);

    QFrame* sep = new QFrame(this);
    sep->setFrameShape(QFrame::HLine);
    sep->setFrameShadow(QFrame::Sunken);
    mainLayout->addWidget(sep);

    QGroupBox* actionsGroup = new QGroupBox("Actions", this);
    QVBoxLayout* actLayout = new QVBoxLayout(actionsGroup);

    m_trackForwardBtn = new QPushButton("Track Forward", this);
    m_trackBackwardBtn = new QPushButton("Track Backward", this);
    m_stabilizeBtn = new QPushButton("Analyze & Stabilize", this);
    m_cameraTrackBtn = new QPushButton("3D Camera Track", this);
    m_resetBtn = new QPushButton("Reset Track Point", this);

    actLayout->addWidget(m_trackForwardBtn);
    actLayout->addWidget(m_trackBackwardBtn);
    actLayout->addWidget(m_stabilizeBtn);
    actLayout->addWidget(m_cameraTrackBtn);
    actLayout->addWidget(m_resetBtn);

    connect(m_trackForwardBtn, &QPushButton::clicked, this, &TrackerPanel::onTrackForward);
    connect(m_trackBackwardBtn, &QPushButton::clicked, this, &TrackerPanel::onTrackBackward);
    connect(m_stabilizeBtn, &QPushButton::clicked, this, &TrackerPanel::onStabilize);
    connect(m_cameraTrackBtn, &QPushButton::clicked, this, &TrackerPanel::onCameraTrack);
    connect(m_resetBtn, &QPushButton::clicked, this, &TrackerPanel::onResetTrack);

    mainLayout->addWidget(actionsGroup);

    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setVisible(false);
    mainLayout->addWidget(m_progressBar);

    mainLayout->addStretch();
}

void TrackerPanel::onTrackForward() {
    m_progressBar->setVisible(true);
    m_progressBar->setValue(0);
    emit trackForwardRequested();
}

void TrackerPanel::onTrackBackward() {
    m_progressBar->setVisible(true);
    m_progressBar->setValue(0);
    emit trackBackwardRequested();
}

void TrackerPanel::onStabilize() { emit stabilizeRequested(); }
void TrackerPanel::onCameraTrack() { emit cameraTrackRequested(); }
void TrackerPanel::onResetTrack() { emit resetTrackPointRequested(); }
void TrackerPanel::onTrackTypeChanged(int index) {}

} // namespace FreeEffect
