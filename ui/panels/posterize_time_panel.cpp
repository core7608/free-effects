#include "posterize_time_panel.h"
#include <QVBoxLayout>
#include <QGridLayout>
#include <QFrame>

namespace FreeEffect {

static const char* kSpinStyle =
    "QDoubleSpinBox { background-color: #2a2a2a; color: #cccccc; border: 1px solid #444444; "
    "border-radius: 2px; padding: 2px 4px; font-size: 11px; }"
    "QDoubleSpinBox:focus { border: 1px solid #ffffff; }"
    "QDoubleSpinBox::up-button, QDoubleSpinBox::down-button { width: 0px; height: 0px; }";

static const char* kBtnStyle =
    "QPushButton { background: #2a5a7a; color: #ffffff; border: 1px solid #3a7aaa; "
    "border-radius: 3px; font-size: 11px; padding: 6px 12px; }"
    "QPushButton:hover { background: #3a7aaa; }"
    "QPushButton:pressed { background: #1a4a6a; }";

PosterizeTimePanel::PosterizeTimePanel(QWidget* parent)
    : QWidget(parent) {
    
    setStyleSheet("background-color: #2a2a2a; color: #cccccc; font-size: 11px;");
    
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(6);
    
    // Header
    QLabel* header = new QLabel("Posterize Time");
    header->setStyleSheet("color: #cccccc; font-size: 11px; font-weight: bold;");
    mainLayout->addWidget(header);
    
    QLabel* descLabel = new QLabel("Reduce the frame rate of a layer to create a strobe effect.");
    descLabel->setStyleSheet("color: #888888; font-size: 10px;");
    descLabel->setWordWrap(true);
    mainLayout->addWidget(descLabel);
    
    QFrame* sep1 = new QFrame();
    sep1->setFrameShape(QFrame::HLine);
    sep1->setStyleSheet("color: #333333;");
    mainLayout->addWidget(sep1);
    
    // Frame rate input
    QGridLayout* grid = new QGridLayout();
    grid->setSpacing(4);
    
    QLabel* fpsLabel = new QLabel("Frame Rate:");
    fpsLabel->setStyleSheet("color: #888888;");
    grid->addWidget(fpsLabel, 0, 0);
    
    m_frameRateSpinner = new QDoubleSpinBox();
    m_frameRateSpinner->setRange(0.1, 120.0);
    m_frameRateSpinner->setValue(12.0);
    m_frameRateSpinner->setDecimals(2);
    m_frameRateSpinner->setSuffix(" fps");
    m_frameRateSpinner->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_frameRateSpinner->setStyleSheet(kSpinStyle);
    grid->addWidget(m_frameRateSpinner, 0, 1);
    
    mainLayout->addLayout(grid);
    
    m_applyBtn = new QPushButton("Apply Posterize Time");
    m_applyBtn->setStyleSheet(kBtnStyle);
    connect(m_applyBtn, &QPushButton::clicked, this, [this]() {
        emit applyPosterizeTime(m_frameRateSpinner->value());
    });
    mainLayout->addWidget(m_applyBtn);
    
    mainLayout->addStretch();
}

} // namespace FreeEffect
