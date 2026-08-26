#include "paint_panel.h"
#include <QVBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QColorDialog>

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

PaintPanel::PaintPanel(QWidget* parent)
    : QWidget(parent) {
    
    setStyleSheet("background-color: #2a2a2a; color: #cccccc; font-size: 11px;");
    
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(6);
    
    // Header
    QLabel* header = new QLabel("Paint");
    header->setStyleSheet("color: #cccccc; font-size: 11px; font-weight: bold;");
    mainLayout->addWidget(header);
    
    QGridLayout* grid = new QGridLayout();
    grid->setSpacing(4);
    int row = 0;
    
    // Tool type
    QLabel* toolLabel = new QLabel("Tool:");
    toolLabel->setStyleSheet("color: #888888;");
    grid->addWidget(toolLabel, row, 0);
    
    m_toolCombo = new QComboBox();
    m_toolCombo->addItems({"Brush", "Clone Stamp", "Eraser"});
    m_toolCombo->setStyleSheet(kComboStyle);
    connect(m_toolCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &PaintPanel::paintModeChanged);
    grid->addWidget(m_toolCombo, row, 1);
    row++;
    
    // Blend mode
    QLabel* modeLabel = new QLabel("Mode:");
    modeLabel->setStyleSheet("color: #888888;");
    grid->addWidget(modeLabel, row, 0);
    
    m_modeCombo = new QComboBox();
    m_modeCombo->addItems({"Normal", "Dissolve", "Multiply", "Screen", "Overlay", "Darken", "Lighten"});
    m_modeCombo->setStyleSheet(kComboStyle);
    connect(m_modeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &PaintPanel::strokeModeChanged);
    grid->addWidget(m_modeCombo, row, 1);
    row++;
    
    // Opacity
    QLabel* opacLabel = new QLabel("Opacity:");
    opacLabel->setStyleSheet("color: #888888;");
    grid->addWidget(opacLabel, row, 0);
    
    m_opacitySpinner = new QDoubleSpinBox();
    m_opacitySpinner->setRange(0.0, 100.0);
    m_opacitySpinner->setValue(100.0);
    m_opacitySpinner->setDecimals(1);
    m_opacitySpinner->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_opacitySpinner->setStyleSheet(kSpinStyle);
    grid->addWidget(m_opacitySpinner, row, 1);
    row++;
    
    // Flow
    QLabel* flowLabel = new QLabel("Flow:");
    flowLabel->setStyleSheet("color: #888888;");
    grid->addWidget(flowLabel, row, 0);
    
    m_flowSpinner = new QDoubleSpinBox();
    m_flowSpinner->setRange(0.0, 100.0);
    m_flowSpinner->setValue(100.0);
    m_flowSpinner->setDecimals(1);
    m_flowSpinner->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_flowSpinner->setStyleSheet(kSpinStyle);
    grid->addWidget(m_flowSpinner, row, 1);
    row++;
    
    QFrame* sep1 = new QFrame();
    sep1->setFrameShape(QFrame::HLine);
    sep1->setStyleSheet("color: #333333;");
    grid->addWidget(sep1, row, 0, 1, 2);
    row++;
    
    // Color
    QLabel* colorLabel = new QLabel("Color:");
    colorLabel->setStyleSheet("color: #888888;");
    grid->addWidget(colorLabel, row, 0);
    
    m_colorBtn = new QPushButton();
    m_colorBtn->setFixedSize(32, 18);
    m_colorBtn->setStyleSheet(
        "QPushButton { background: #ffffff; border: 1px solid #555555; border-radius: 2px; }"
        "QPushButton:hover { border: 1px solid #ffffff; }");
    connect(m_colorBtn, &QPushButton::clicked, this, [this]() {
        QColor color = QColorDialog::getColor(Qt::white, this, "Paint Color");
        if (color.isValid()) {
            m_colorBtn->setStyleSheet(
                QString("QPushButton { background: %1; border: 1px solid #555555; border-radius: 2px; }"
                        "QPushButton:hover { border: 1px solid #ffffff; }").arg(color.name()));
        }
    });
    grid->addWidget(m_colorBtn, row, 1);
    row++;
    
    // Channel
    QLabel* channelLabel = new QLabel("Channel:");
    channelLabel->setStyleSheet("color: #888888;");
    grid->addWidget(channelLabel, row, 0);
    
    m_channelCombo = new QComboBox();
    m_channelCombo->addItems({"RGB", "Red", "Green", "Blue", "Alpha"});
    m_channelCombo->setStyleSheet(kComboStyle);
    grid->addWidget(m_channelCombo, row, 1);
    
    mainLayout->addLayout(grid);
    mainLayout->addStretch();
}

} // namespace FreeEffect
