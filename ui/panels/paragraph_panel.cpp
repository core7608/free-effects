#include "paragraph_panel.h"
#include <QVBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QButtonGroup>

namespace FreeEffect {

static const char* kSpinStyle =
    "QDoubleSpinBox { background-color: #2a2a2a; color: #cccccc; border: 1px solid #444444; "
    "border-radius: 2px; padding: 2px 4px; font-size: 11px; }"
    "QDoubleSpinBox:focus { border: 1px solid #ffffff; }"
    "QDoubleSpinBox::up-button, QDoubleSpinBox::down-button { width: 0px; height: 0px; }";

static const char* kAlignBtnStyle =
    "QPushButton { background: #3c3c3c; color: #cccccc; border: 1px solid #555555; "
    "border-radius: 3px; font-size: 10px; padding: 4px 8px; }"
    "QPushButton:hover { background: #4a4a4a; }"
    "QPushButton:checked { background: #00d4ff; color: #000000; border: 1px solid #00d4ff; }";

ParagraphPanel::ParagraphPanel(QWidget* parent)
    : QWidget(parent) {
    
    setStyleSheet("background-color: #2a2a2a; color: #cccccc; font-size: 11px;");
    
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(6);
    
    // Header
    QLabel* header = new QLabel("Paragraph");
    header->setStyleSheet("color: #cccccc; font-size: 11px; font-weight: bold;");
    mainLayout->addWidget(header);
    
    // Justification buttons
    QLabel* alignLabel = new QLabel("Justification:");
    alignLabel->setStyleSheet("color: #888888;");
    mainLayout->addWidget(alignLabel);
    
    // First row: left, center, right
    QWidget* row1 = new QWidget();
    QHBoxLayout* row1Layout = new QHBoxLayout(row1);
    row1Layout->setContentsMargins(0, 0, 0, 0);
    row1Layout->setSpacing(4);
    
    QButtonGroup* alignGroup = new QButtonGroup(this);
    alignGroup->setExclusive(true);
    
    m_leftAlignBtn = new QPushButton("Left");
    m_leftAlignBtn->setCheckable(true);
    m_leftAlignBtn->setChecked(true);
    m_leftAlignBtn->setStyleSheet(kAlignBtnStyle);
    alignGroup->addButton(m_leftAlignBtn, 0);
    connect(m_leftAlignBtn, &QPushButton::clicked, this, [this]() { emit justificationChanged(0); });
    row1Layout->addWidget(m_leftAlignBtn);
    
    m_centerAlignBtn = new QPushButton("Center");
    m_centerAlignBtn->setCheckable(true);
    m_centerAlignBtn->setStyleSheet(kAlignBtnStyle);
    alignGroup->addButton(m_centerAlignBtn, 1);
    connect(m_centerAlignBtn, &QPushButton::clicked, this, [this]() { emit justificationChanged(1); });
    row1Layout->addWidget(m_centerAlignBtn);
    
    m_rightAlignBtn = new QPushButton("Right");
    m_rightAlignBtn->setCheckable(true);
    m_rightAlignBtn->setStyleSheet(kAlignBtnStyle);
    alignGroup->addButton(m_rightAlignBtn, 2);
    connect(m_rightAlignBtn, &QPushButton::clicked, this, [this]() { emit justificationChanged(2); });
    row1Layout->addWidget(m_rightAlignBtn);
    
    mainLayout->addWidget(row1);
    
    // Second row: last line variants
    QWidget* row2 = new QWidget();
    QHBoxLayout* row2Layout = new QHBoxLayout(row2);
    row2Layout->setContentsMargins(0, 0, 0, 0);
    row2Layout->setSpacing(4);
    
    m_lastLeftBtn = new QPushButton("Last L");
    m_lastLeftBtn->setCheckable(true);
    m_lastLeftBtn->setStyleSheet(kAlignBtnStyle);
    alignGroup->addButton(m_lastLeftBtn, 3);
    connect(m_lastLeftBtn, &QPushButton::clicked, this, [this]() { emit justificationChanged(3); });
    row2Layout->addWidget(m_lastLeftBtn);
    
    m_lastCenterBtn = new QPushButton("Last C");
    m_lastCenterBtn->setCheckable(true);
    m_lastCenterBtn->setStyleSheet(kAlignBtnStyle);
    alignGroup->addButton(m_lastCenterBtn, 4);
    connect(m_lastCenterBtn, &QPushButton::clicked, this, [this]() { emit justificationChanged(4); });
    row2Layout->addWidget(m_lastCenterBtn);
    
    m_lastRightBtn = new QPushButton("Last R");
    m_lastRightBtn->setCheckable(true);
    m_lastRightBtn->setStyleSheet(kAlignBtnStyle);
    alignGroup->addButton(m_lastRightBtn, 5);
    connect(m_lastRightBtn, &QPushButton::clicked, this, [this]() { emit justificationChanged(5); });
    row2Layout->addWidget(m_lastRightBtn);
    
    mainLayout->addWidget(row2);
    
    QFrame* sep1 = new QFrame();
    sep1->setFrameShape(QFrame::HLine);
    sep1->setStyleSheet("color: #333333;");
    mainLayout->addWidget(sep1);
    
    // Indent and margins
    QGridLayout* grid = new QGridLayout();
    grid->setSpacing(4);
    int row = 0;
    
    QLabel* indentLabel = new QLabel("Indent:");
    indentLabel->setStyleSheet("color: #888888;");
    grid->addWidget(indentLabel, row, 0);
    
    m_indentSpinner = new QDoubleSpinBox();
    m_indentSpinner->setRange(-1000.0, 1000.0);
    m_indentSpinner->setValue(0.0);
    m_indentSpinner->setDecimals(1);
    m_indentSpinner->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_indentSpinner->setStyleSheet(kSpinStyle);
    connect(m_indentSpinner, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
            this, &ParagraphPanel::indentChanged);
    grid->addWidget(m_indentSpinner, row, 1);
    row++;
    
    QLabel* leftLabel = new QLabel("Left Margin:");
    leftLabel->setStyleSheet("color: #888888;");
    grid->addWidget(leftLabel, row, 0);
    
    m_leftMarginSpinner = new QDoubleSpinBox();
    m_leftMarginSpinner->setRange(0.0, 10000.0);
    m_leftMarginSpinner->setValue(0.0);
    m_leftMarginSpinner->setDecimals(1);
    m_leftMarginSpinner->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_leftMarginSpinner->setStyleSheet(kSpinStyle);
    connect(m_leftMarginSpinner, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
            this, &ParagraphPanel::leftMarginChanged);
    grid->addWidget(m_leftMarginSpinner, row, 1);
    row++;
    
    QLabel* rightLabel = new QLabel("Right Margin:");
    rightLabel->setStyleSheet("color: #888888;");
    grid->addWidget(rightLabel, row, 0);
    
    m_rightMarginSpinner = new QDoubleSpinBox();
    m_rightMarginSpinner->setRange(0.0, 10000.0);
    m_rightMarginSpinner->setValue(0.0);
    m_rightMarginSpinner->setDecimals(1);
    m_rightMarginSpinner->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_rightMarginSpinner->setStyleSheet(kSpinStyle);
    connect(m_rightMarginSpinner, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
            this, &ParagraphPanel::rightMarginChanged);
    grid->addWidget(m_rightMarginSpinner, row, 1);
    row++;
    
    QFrame* sep2 = new QFrame();
    sep2->setFrameShape(QFrame::HLine);
    sep2->setStyleSheet("color: #333333;");
    grid->addWidget(sep2, row, 0, 1, 2);
    row++;
    
    QLabel* beforeLabel = new QLabel("Space Before:");
    beforeLabel->setStyleSheet("color: #888888;");
    grid->addWidget(beforeLabel, row, 0);
    
    m_spaceBeforeSpinner = new QDoubleSpinBox();
    m_spaceBeforeSpinner->setRange(0.0, 10000.0);
    m_spaceBeforeSpinner->setValue(0.0);
    m_spaceBeforeSpinner->setDecimals(1);
    m_spaceBeforeSpinner->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_spaceBeforeSpinner->setStyleSheet(kSpinStyle);
    connect(m_spaceBeforeSpinner, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
            this, &ParagraphPanel::spaceBeforeChanged);
    grid->addWidget(m_spaceBeforeSpinner, row, 1);
    row++;
    
    QLabel* afterLabel = new QLabel("Space After:");
    afterLabel->setStyleSheet("color: #888888;");
    grid->addWidget(afterLabel, row, 0);
    
    m_spaceAfterSpinner = new QDoubleSpinBox();
    m_spaceAfterSpinner->setRange(0.0, 10000.0);
    m_spaceAfterSpinner->setValue(0.0);
    m_spaceAfterSpinner->setDecimals(1);
    m_spaceAfterSpinner->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_spaceAfterSpinner->setStyleSheet(kSpinStyle);
    connect(m_spaceAfterSpinner, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
            this, &ParagraphPanel::spaceAfterChanged);
    grid->addWidget(m_spaceAfterSpinner, row, 1);
    
    mainLayout->addLayout(grid);
    mainLayout->addStretch();
}

void ParagraphPanel::setJustification(int mode) {
    switch (mode) {
        case 0: m_leftAlignBtn->setChecked(true); break;
        case 1: m_centerAlignBtn->setChecked(true); break;
        case 2: m_rightAlignBtn->setChecked(true); break;
        case 3: m_lastLeftBtn->setChecked(true); break;
        case 4: m_lastCenterBtn->setChecked(true); break;
        case 5: m_lastRightBtn->setChecked(true); break;
    }
}

void ParagraphPanel::setIndent(double indent) {
    m_indentSpinner->setValue(indent);
}

void ParagraphPanel::setLeftMargin(double margin) {
    m_leftMarginSpinner->setValue(margin);
}

void ParagraphPanel::setRightMargin(double margin) {
    m_rightMarginSpinner->setValue(margin);
}

void ParagraphPanel::setSpaceBefore(double space) {
    m_spaceBeforeSpinner->setValue(space);
}

void ParagraphPanel::setSpaceAfter(double space) {
    m_spaceAfterSpinner->setValue(space);
}

} // namespace FreeEffect
