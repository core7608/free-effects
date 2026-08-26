#include "wiggler_panel.h"
#include <QVBoxLayout>
#include <QGridLayout>
#include <QFrame>

namespace FreeEffect {

static const char* kSpinStyle =
    "QDoubleSpinBox { background-color: #2a2a2a; color: #cccccc; border: 1px solid #444444; "
    "border-radius: 2px; padding: 2px 4px; font-size: 11px; }"
    "QDoubleSpinBox:focus { border: 1px solid #ffffff; }"
    "QDoubleSpinBox::up-button, QDoubleSpinBox::down-button { width: 0px; height: 0px; }";

static const char* kComboStyle =
    "QComboBox { background-color: #2a2a2a; color: #cccccc; border: 1px solid #444444; "
    "border-radius: 2px; padding: 2px 4px; font-size: 11px; }"
    "QComboBox::drop-down { border: none; }"
    "QComboBox QAbstractItemView { background: #2a2a2a; color: #cccccc; selection-background-color: #2a5a7a; }";

static const char* kBtnStyle =
    "QPushButton { background: #2a5a7a; color: #ffffff; border: 1px solid #3a7aaa; "
    "border-radius: 3px; font-size: 11px; padding: 6px 12px; }"
    "QPushButton:hover { background: #3a7aaa; }"
    "QPushButton:pressed { background: #1a4a6a; }";

static const char* kSmoothBtnStyle =
    "QPushButton { background: #5a3a7a; color: #ffffff; border: 1px solid #7a5a9a; "
    "border-radius: 3px; font-size: 11px; padding: 6px 12px; }"
    "QPushButton:hover { background: #7a5a9a; }"
    "QPushButton:pressed { background: #4a2a6a; }";

WigglerPanel::WigglerPanel(QWidget* parent)
    : QWidget(parent) {
    
    setStyleSheet("background-color: #2a2a2a; color: #cccccc; font-size: 11px;");
    
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(6);
    
    // Header
    QLabel* header = new QLabel("Wiggler");
    header->setStyleSheet("color: #cccccc; font-size: 11px; font-weight: bold;");
    mainLayout->addWidget(header);
    
    // Wiggle parameters
    QGridLayout* grid = new QGridLayout();
    grid->setSpacing(4);
    int row = 0;
    
    QLabel* freqLabel = new QLabel("Frequency:");
    freqLabel->setStyleSheet("color: #888888;");
    grid->addWidget(freqLabel, row, 0);
    
    m_frequencySpinner = new QDoubleSpinBox();
    m_frequencySpinner->setRange(0.1, 1000.0);
    m_frequencySpinner->setValue(1.0);
    m_frequencySpinner->setDecimals(2);
    m_frequencySpinner->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_frequencySpinner->setStyleSheet(kSpinStyle);
    grid->addWidget(m_frequencySpinner, row, 1);
    row++;
    
    QLabel* ampLabel = new QLabel("Amplitude:");
    ampLabel->setStyleSheet("color: #888888;");
    grid->addWidget(ampLabel, row, 0);
    
    m_amplitudeSpinner = new QDoubleSpinBox();
    m_amplitudeSpinner->setRange(0.0, 10000.0);
    m_amplitudeSpinner->setValue(50.0);
    m_amplitudeSpinner->setDecimals(1);
    m_amplitudeSpinner->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_amplitudeSpinner->setStyleSheet(kSpinStyle);
    grid->addWidget(m_amplitudeSpinner, row, 1);
    row++;
    
    QLabel* dimLabel = new QLabel("Dimensions:");
    dimLabel->setStyleSheet("color: #888888;");
    grid->addWidget(dimLabel, row, 0);
    
    m_dimensionsCombo = new QComboBox();
    m_dimensionsCombo->addItems({"Horizontal", "Vertical", "Both"});
    m_dimensionsCombo->setStyleSheet(kComboStyle);
    grid->addWidget(m_dimensionsCombo, row, 1);
    
    mainLayout->addLayout(grid);
    
    QFrame* sep1 = new QFrame();
    sep1->setFrameShape(QFrame::HLine);
    sep1->setStyleSheet("color: #333333;");
    mainLayout->addWidget(sep1);
    
    // Affect checkboxes
    QLabel* affectLabel = new QLabel("Affect:");
    affectLabel->setStyleSheet("color: #888888; font-size: 10px;");
    mainLayout->addWidget(affectLabel);
    
    m_positionCheck = new QCheckBox("Position");
    m_positionCheck->setChecked(true);
    m_positionCheck->setStyleSheet("color: #cccccc;");
    mainLayout->addWidget(m_positionCheck);
    
    m_rotationCheck = new QCheckBox("Rotation");
    m_rotationCheck->setChecked(false);
    m_rotationCheck->setStyleSheet("color: #cccccc;");
    mainLayout->addWidget(m_rotationCheck);
    
    m_scaleCheck = new QCheckBox("Scale");
    m_scaleCheck->setChecked(false);
    m_scaleCheck->setStyleSheet("color: #cccccc;");
    mainLayout->addWidget(m_scaleCheck);
    
    QFrame* sep2 = new QFrame();
    sep2->setFrameShape(QFrame::HLine);
    sep2->setStyleSheet("color: #333333;");
    mainLayout->addWidget(sep2);
    
    // Apply buttons
    m_applyBtn = new QPushButton("Apply Wiggle");
    m_applyBtn->setStyleSheet(kBtnStyle);
    connect(m_applyBtn, &QPushButton::clicked, this, [this]() {
        emit applyWiggle(
            m_frequencySpinner->value(),
            m_amplitudeSpinner->value(),
            m_dimensionsCombo->currentIndex(),
            m_positionCheck->isChecked(),
            m_rotationCheck->isChecked(),
            m_scaleCheck->isChecked()
        );
    });
    mainLayout->addWidget(m_applyBtn);
    
    m_smoothApplyBtn = new QPushButton("Apply Smooth Wiggle");
    m_smoothApplyBtn->setStyleSheet(kSmoothBtnStyle);
    connect(m_smoothApplyBtn, &QPushButton::clicked, this, [this]() {
        emit applySmoothWiggle(m_frequencySpinner->value(), m_amplitudeSpinner->value());
    });
    mainLayout->addWidget(m_smoothApplyBtn);
    
    mainLayout->addStretch();
}

} // namespace FreeEffect
