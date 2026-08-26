#include "motion_sketch_panel.h"
#include <QVBoxLayout>
#include <QGridLayout>
#include <QFrame>

namespace FreeEffect {

static const char* kSpinStyle =
    "QDoubleSpinBox { background-color: #2a2a2a; color: #cccccc; border: 1px solid #444444; "
    "border-radius: 2px; padding: 2px 4px; font-size: 11px; }"
    "QDoubleSpinBox:focus { border: 1px solid #ffffff; }"
    "QDoubleSpinBox::up-button, QDoubleSpinBox::down-button { width: 0px; height: 0px; }";

static const char* kStartBtnStyle =
    "QPushButton { background: #00aa44; color: #ffffff; border: 1px solid #00cc55; "
    "border-radius: 3px; font-size: 11px; font-weight: bold; padding: 6px 12px; }"
    "QPushButton:hover { background: #00cc55; }"
    "QPushButton:pressed { background: #008833; }";

static const char* kStopBtnStyle =
    "QPushButton { background: #cc3333; color: #ffffff; border: 1px solid #ee4444; "
    "border-radius: 3px; font-size: 11px; font-weight: bold; padding: 6px 12px; }"
    "QPushButton:hover { background: #ee4444; }"
    "QPushButton:pressed { background: #aa2222; }";

static const char* kBtnStyle =
    "QPushButton { background: #2a5a7a; color: #ffffff; border: 1px solid #3a7aaa; "
    "border-radius: 3px; font-size: 11px; padding: 6px 12px; }"
    "QPushButton:hover { background: #3a7aaa; }"
    "QPushButton:pressed { background: #1a4a6a; }";

MotionSketchPanel::MotionSketchPanel(QWidget* parent)
    : QWidget(parent) {
    
    setStyleSheet("background-color: #2a2a2a; color: #cccccc; font-size: 11px;");
    
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(6);
    
    // Header
    QLabel* header = new QLabel("Motion Sketch");
    header->setStyleSheet("color: #cccccc; font-size: 11px; font-weight: bold;");
    mainLayout->addWidget(header);
    
    // Status
    m_statusLabel = new QLabel("Ready");
    m_statusLabel->setStyleSheet("color: #888888; font-size: 10px; padding: 2px 0px;");
    mainLayout->addWidget(m_statusLabel);
    
    // Capture rate
    QGridLayout* grid = new QGridLayout();
    grid->setSpacing(4);
    
    QLabel* rateLabel = new QLabel("Capture Rate:");
    rateLabel->setStyleSheet("color: #888888;");
    grid->addWidget(rateLabel, 0, 0);
    
    m_captureRateSpinner = new QDoubleSpinBox();
    m_captureRateSpinner->setRange(1.0, 120.0);
    m_captureRateSpinner->setValue(30.0);
    m_captureRateSpinner->setDecimals(0);
    m_captureRateSpinner->setSuffix(" fps");
    m_captureRateSpinner->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_captureRateSpinner->setStyleSheet(kSpinStyle);
    grid->addWidget(m_captureRateSpinner, 0, 1);
    
    mainLayout->addLayout(grid);
    
    // Include options
    m_includeRotationCheck = new QCheckBox("Include Rotation");
    m_includeRotationCheck->setChecked(false);
    m_includeRotationCheck->setStyleSheet("color: #cccccc;");
    mainLayout->addWidget(m_includeRotationCheck);
    
    m_includeScaleCheck = new QCheckBox("Include Scale");
    m_includeScaleCheck->setChecked(false);
    m_includeScaleCheck->setStyleSheet("color: #cccccc;");
    mainLayout->addWidget(m_includeScaleCheck);
    
    QFrame* sep1 = new QFrame();
    sep1->setFrameShape(QFrame::HLine);
    sep1->setStyleSheet("color: #333333;");
    mainLayout->addWidget(sep1);
    
    // Capture buttons
    m_startCaptureBtn = new QPushButton("Start Capture");
    m_startCaptureBtn->setStyleSheet(kStartBtnStyle);
    connect(m_startCaptureBtn, &QPushButton::clicked, this, [this]() {
        m_statusLabel->setText("Capturing...");
        m_statusLabel->setStyleSheet("color: #00cc55; font-size: 10px; padding: 2px 0px;");
        emit startCapture();
    });
    mainLayout->addWidget(m_startCaptureBtn);
    
    m_stopCaptureBtn = new QPushButton("Stop Capture");
    m_stopCaptureBtn->setStyleSheet(kStopBtnStyle);
    connect(m_stopCaptureBtn, &QPushButton::clicked, this, [this]() {
        m_statusLabel->setText("Capture stopped");
        m_statusLabel->setStyleSheet("color: #cc8800; font-size: 10px; padding: 2px 0px;");
        emit stopCapture();
    });
    mainLayout->addWidget(m_stopCaptureBtn);
    
    QFrame* sep2 = new QFrame();
    sep2->setFrameShape(QFrame::HLine);
    sep2->setStyleSheet("color: #333333;");
    mainLayout->addWidget(sep2);
    
    // Apply buttons
    m_applyBtn = new QPushButton("Apply to Position");
    m_applyBtn->setStyleSheet(kBtnStyle);
    connect(m_applyBtn, &QPushButton::clicked, this, [this]() {
        m_statusLabel->setText("Applied to position");
        m_statusLabel->setStyleSheet("color: #cccccc; font-size: 10px; padding: 2px 0px;");
        emit applyToPosition();
    });
    mainLayout->addWidget(m_applyBtn);
    
    m_createLayerBtn = new QPushButton("Create New Layer");
    m_createLayerBtn->setStyleSheet(kBtnStyle);
    connect(m_createLayerBtn, &QPushButton::clicked, this, &MotionSketchPanel::createNewLayer);
    mainLayout->addWidget(m_createLayerBtn);
    
    mainLayout->addStretch();
}

} // namespace FreeEffect
