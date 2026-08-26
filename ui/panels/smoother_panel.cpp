#include "smoother_panel.h"
#include <QVBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QSpinBox>

namespace FreeEffect {

static const char* kSpinStyle =
    "QDoubleSpinBox { background-color: #2a2a2a; color: #cccccc; border: 1px solid #444444; "
    "border-radius: 2px; padding: 2px 4px; font-size: 11px; }"
    "QDoubleSpinBox:focus { border: 1px solid #ffffff; }"
    "QDoubleSpinBox::up-button, QDoubleSpinBox::down-button { width: 0px; height: 0px; }";

static const char* kSpinBoxStyle =
    "QSpinBox { background-color: #2a2a2a; color: #cccccc; border: 1px solid #444444; "
    "border-radius: 2px; padding: 2px 4px; font-size: 11px; }"
    "QSpinBox:focus { border: 1px solid #ffffff; }"
    "QSpinBox::up-button, QSpinBox::down-button { width: 0px; height: 0px; }";

static const char* kBtnStyle =
    "QPushButton { background: #2a5a7a; color: #ffffff; border: 1px solid #3a7aaa; "
    "border-radius: 3px; font-size: 11px; padding: 6px 12px; }"
    "QPushButton:hover { background: #3a7aaa; }"
    "QPushButton:pressed { background: #1a4a6a; }";

SmootherPanel::SmootherPanel(QWidget* parent)
    : QWidget(parent) {
    
    setStyleSheet("background-color: #2a2a2a; color: #cccccc; font-size: 11px;");
    
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(6);
    
    // Header
    QLabel* header = new QLabel("Smoother");
    header->setStyleSheet("color: #cccccc; font-size: 11px; font-weight: bold;");
    mainLayout->addWidget(header);
    
    // Spatial smoother section
    QLabel* smoothLabel = new QLabel("Motion Smoother");
    smoothLabel->setStyleSheet("color: #888888; font-size: 10px;");
    mainLayout->addWidget(smoothLabel);
    
    QGridLayout* smoothGrid = new QGridLayout();
    smoothGrid->setSpacing(4);
    
    QLabel* spatialLabel = new QLabel("Spatial Precision:");
    spatialLabel->setStyleSheet("color: #888888;");
    smoothGrid->addWidget(spatialLabel, 0, 0);
    
    m_spatialPrecisionSpinner = new QDoubleSpinBox();
    m_spatialPrecisionSpinner->setRange(0.0, 1000.0);
    m_spatialPrecisionSpinner->setValue(1.0);
    m_spatialPrecisionSpinner->setDecimals(1);
    m_spatialPrecisionSpinner->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_spatialPrecisionSpinner->setStyleSheet(kSpinStyle);
    smoothGrid->addWidget(m_spatialPrecisionSpinner, 0, 1);
    
    QLabel* temporalLabel = new QLabel("Temporal Precision:");
    temporalLabel->setStyleSheet("color: #888888;");
    smoothGrid->addWidget(temporalLabel, 1, 0);
    
    m_temporalPrecisionSpinner = new QDoubleSpinBox();
    m_temporalPrecisionSpinner->setRange(0.0, 1000.0);
    m_temporalPrecisionSpinner->setValue(1.0);
    m_temporalPrecisionSpinner->setDecimals(1);
    m_temporalPrecisionSpinner->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_temporalPrecisionSpinner->setStyleSheet(kSpinStyle);
    smoothGrid->addWidget(m_temporalPrecisionSpinner, 1, 1);
    
    mainLayout->addLayout(smoothGrid);
    
    m_applySmoothBtn = new QPushButton("Apply Smoother");
    m_applySmoothBtn->setStyleSheet(kBtnStyle);
    connect(m_applySmoothBtn, &QPushButton::clicked, this, [this]() {
        emit applySmoother(m_spatialPrecisionSpinner->value(), 
                          m_temporalPrecisionSpinner->value());
    });
    mainLayout->addWidget(m_applySmoothBtn);
    
    QFrame* sep1 = new QFrame();
    sep1->setFrameShape(QFrame::HLine);
    sep1->setStyleSheet("color: #333333;");
    mainLayout->addWidget(sep1);
    
    // Simplifier section
    QLabel* simplifyLabel = new QLabel("Keyframe Simplifier");
    simplifyLabel->setStyleSheet("color: #888888; font-size: 10px;");
    mainLayout->addWidget(simplifyLabel);
    
    QGridLayout* simplifyGrid = new QGridLayout();
    simplifyGrid->setSpacing(4);
    
    QLabel* tolLabel = new QLabel("Tolerance:");
    tolLabel->setStyleSheet("color: #888888;");
    simplifyGrid->addWidget(tolLabel, 0, 0);
    
    m_toleranceSpinner = new QDoubleSpinBox();
    m_toleranceSpinner->setRange(0.0, 1000.0);
    m_toleranceSpinner->setValue(1.0);
    m_toleranceSpinner->setDecimals(2);
    m_toleranceSpinner->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_toleranceSpinner->setStyleSheet(kSpinStyle);
    simplifyGrid->addWidget(m_toleranceSpinner, 0, 1);
    
    QLabel* maxLabel = new QLabel("Max Keyframes:");
    maxLabel->setStyleSheet("color: #888888;");
    simplifyGrid->addWidget(maxLabel, 1, 0);
    
    m_maxKeyframesSpinner = new QSpinBox();
    m_maxKeyframesSpinner->setRange(1, 10000);
    m_maxKeyframesSpinner->setValue(100);
    m_maxKeyframesSpinner->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_maxKeyframesSpinner->setStyleSheet(kSpinBoxStyle);
    simplifyGrid->addWidget(m_maxKeyframesSpinner, 1, 1);
    
    mainLayout->addLayout(simplifyGrid);
    
    m_applySimplifyBtn = new QPushButton("Apply Simplifier");
    m_applySimplifyBtn->setStyleSheet(kBtnStyle);
    connect(m_applySimplifyBtn, &QPushButton::clicked, this, [this]() {
        emit applySimplifier(m_toleranceSpinner->value(), 
                            m_maxKeyframesSpinner->value());
    });
    mainLayout->addWidget(m_applySimplifyBtn);
    
    mainLayout->addStretch();
}

} // namespace FreeEffect
