#include "brushes_panel.h"
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

BrushesPanel::BrushesPanel(QWidget* parent)
    : QWidget(parent) {
    
    setStyleSheet("background-color: #2a2a2a; color: #cccccc; font-size: 11px;");
    
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(6);
    
    // Header
    QLabel* header = new QLabel("Brushes");
    header->setStyleSheet("color: #cccccc; font-size: 11px; font-weight: bold;");
    mainLayout->addWidget(header);
    
    // Brush presets list
    m_brushList = new QListWidget();
    m_brushList->setFixedHeight(120);
    m_brushList->setStyleSheet(
        "QListWidget { background: #1a1a1a; color: #cccccc; border: 1px solid #333333; "
        "font-size: 11px; }"
        "QListWidget::item { padding: 3px 6px; }"
        "QListWidget::item:selected { background: #2a5a7a; }"
        "QListWidget::item:hover { background: #2a2a2a; }"
    );
    connect(m_brushList, &QListWidget::currentRowChanged, this, [this](int row) {
        if (row >= 0 && row < static_cast<int>(m_brushes.size())) {
            const auto& brush = m_brushes[row];
            m_sizeSpinner->setValue(brush.size);
            m_hardnessSpinner->setValue(brush.hardness);
            m_opacitySpinner->setValue(brush.opacity);
            m_flowSpinner->setValue(brush.flow);
            m_spacingSpinner->setValue(brush.spacing);
            emit brushChanged(brush);
        }
    });
    mainLayout->addWidget(m_brushList);
    
    QFrame* sep1 = new QFrame();
    sep1->setFrameShape(QFrame::HLine);
    sep1->setStyleSheet("color: #333333;");
    mainLayout->addWidget(sep1);
    
    // Brush properties
    QLabel* propsLabel = new QLabel("Brush Settings");
    propsLabel->setStyleSheet("color: #888888; font-size: 10px;");
    mainLayout->addWidget(propsLabel);
    
    QGridLayout* grid = new QGridLayout();
    grid->setSpacing(4);
    int row = 0;
    
    // Size
    QLabel* sizeLabel = new QLabel("Size:");
    sizeLabel->setStyleSheet("color: #888888;");
    grid->addWidget(sizeLabel, row, 0);
    
    m_sizeSpinner = new QDoubleSpinBox();
    m_sizeSpinner->setRange(1.0, 5000.0);
    m_sizeSpinner->setValue(10.0);
    m_sizeSpinner->setDecimals(1);
    m_sizeSpinner->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_sizeSpinner->setStyleSheet(kSpinStyle);
    connect(m_sizeSpinner, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double) {
        emit brushChanged(getCurrentBrush());
    });
    grid->addWidget(m_sizeSpinner, row, 1);
    row++;
    
    // Hardness
    QLabel* hardLabel = new QLabel("Hardness:");
    hardLabel->setStyleSheet("color: #888888;");
    grid->addWidget(hardLabel, row, 0);
    
    m_hardnessSpinner = new QDoubleSpinBox();
    m_hardnessSpinner->setRange(0.0, 100.0);
    m_hardnessSpinner->setValue(100.0);
    m_hardnessSpinner->setDecimals(1);
    m_hardnessSpinner->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_hardnessSpinner->setStyleSheet(kSpinStyle);
    connect(m_hardnessSpinner, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double) {
        emit brushChanged(getCurrentBrush());
    });
    grid->addWidget(m_hardnessSpinner, row, 1);
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
    connect(m_opacitySpinner, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double) {
        emit brushChanged(getCurrentBrush());
    });
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
    connect(m_flowSpinner, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double) {
        emit brushChanged(getCurrentBrush());
    });
    grid->addWidget(m_flowSpinner, row, 1);
    row++;
    
    // Spacing
    QLabel* spacLabel = new QLabel("Spacing:");
    spacLabel->setStyleSheet("color: #888888;");
    grid->addWidget(spacLabel, row, 0);
    
    m_spacingSpinner = new QSpinBox();
    m_spacingSpinner->setRange(1, 1000);
    m_spacingSpinner->setValue(25);
    m_spacingSpinner->setSuffix("%");
    m_spacingSpinner->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_spacingSpinner->setStyleSheet(kSpinBoxStyle);
    connect(m_spacingSpinner, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int) {
        emit brushChanged(getCurrentBrush());
    });
    grid->addWidget(m_spacingSpinner, row, 1);
    
    mainLayout->addLayout(grid);
    mainLayout->addStretch();
    
    loadDefaultBrushes();
}

void BrushesPanel::loadDefaultBrushes() {
    m_brushes.clear();
    m_brushList->clear();
    
    m_brushes.push_back({"Soft Round", 20.0, 0.0, 100.0, 100.0, 25});
    m_brushes.push_back({"Hard Round", 10.0, 100.0, 100.0, 100.0, 25});
    m_brushes.push_back({"Soft Flat", 30.0, 0.0, 100.0, 100.0, 25});
    m_brushes.push_back({"Hard Flat", 25.0, 100.0, 100.0, 100.0, 25});
    m_brushes.push_back({"Calligraphy 1", 15.0, 100.0, 100.0, 100.0, 25});
    m_brushes.push_back({"Spatter", 40.0, 70.0, 80.0, 80.0, 50});
    m_brushes.push_back({"Airbrush Soft", 50.0, 0.0, 50.0, 30.0, 10});
    m_brushes.push_back({"Chalk", 35.0, 50.0, 90.0, 100.0, 30});
    m_brushes.push_back({"Charcoal", 20.0, 30.0, 85.0, 90.0, 35});
    m_brushes.push_back({"Small Round", 5.0, 100.0, 100.0, 100.0, 25});
    
    for (const auto& brush : m_brushes) {
        m_brushList->addItem(brush.name);
    }
    
    if (!m_brushes.empty()) {
        m_brushList->setCurrentRow(0);
    }
}

BrushPreset BrushesPanel::getCurrentBrush() const {
    BrushPreset brush;
    brush.size = m_sizeSpinner->value();
    brush.hardness = m_hardnessSpinner->value();
    brush.opacity = m_opacitySpinner->value();
    brush.flow = m_flowSpinner->value();
    brush.spacing = m_spacingSpinner->value();
    
    int row = m_brushList->currentRow();
    if (row >= 0 && row < static_cast<int>(m_brushes.size())) {
        brush.name = m_brushes[row].name;
    }
    
    return brush;
}

} // namespace FreeEffect
