#include "character_panel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QFontDatabase>

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

static const char* kColorBtnStyle =
    "QPushButton { border: 1px solid #555555; border-radius: 2px; }"
    "QPushButton:hover { border: 1px solid #ffffff; }";

static const char* kSmallBtnStyle =
    "QPushButton { background: #3c3c3c; color: #cccccc; border: 1px solid #555555; "
    "border-radius: 3px; font-size: 10px; padding: 2px 6px; }"
    "QPushButton:hover { background: #4a4a4a; }";

CharacterPanel::CharacterPanel(QWidget* parent)
    : QWidget(parent) {
    
    setStyleSheet("background-color: #2a2a2a; color: #cccccc; font-size: 11px;");
    
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(6);
    
    // Header
    QLabel* header = new QLabel("Character");
    header->setStyleSheet("color: #cccccc; font-size: 11px; font-weight: bold;");
    mainLayout->addWidget(header);
    
    QGridLayout* grid = new QGridLayout();
    grid->setSpacing(4);
    int row = 0;
    
    // Font family
    QLabel* fontLabel = new QLabel("Font:");
    fontLabel->setStyleSheet("color: #888888;");
    grid->addWidget(fontLabel, row, 0);
    
    m_fontCombo = new QFontComboBox();
    m_fontCombo->setCurrentFont(QFont("Arial"));
    m_fontCombo->setStyleSheet(kComboStyle);
    connect(m_fontCombo, &QFontComboBox::currentFontChanged, this, &CharacterPanel::fontChanged);
    grid->addWidget(m_fontCombo, row, 1, 1, 2);
    row++;
    
    // Font style
    QLabel* styleLabel = new QLabel("Style:");
    styleLabel->setStyleSheet("color: #888888;");
    grid->addWidget(styleLabel, row, 0);
    
    m_fontStyleCombo = new QComboBox();
    m_fontStyleCombo->addItems({"Regular", "Bold", "Italic", "Bold Italic"});
    m_fontStyleCombo->setStyleSheet(kComboStyle);
    grid->addWidget(m_fontStyleCombo, row, 1, 1, 2);
    row++;
    
    // Font size
    QLabel* sizeLabel = new QLabel("Size:");
    sizeLabel->setStyleSheet("color: #888888;");
    grid->addWidget(sizeLabel, row, 0);
    
    m_sizeSpinner = new QDoubleSpinBox();
    m_sizeSpinner->setRange(1.0, 1296.0);
    m_sizeSpinner->setValue(36.0);
    m_sizeSpinner->setDecimals(1);
    m_sizeSpinner->setSingleStep(1.0);
    m_sizeSpinner->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_sizeSpinner->setStyleSheet(kSpinStyle);
    connect(m_sizeSpinner, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
            this, &CharacterPanel::fontSizeChanged);
    grid->addWidget(m_sizeSpinner, row, 1);
    
    QLabel* ptLabel = new QLabel("pt");
    ptLabel->setStyleSheet("color: #888888;");
    grid->addWidget(ptLabel, row, 2);
    row++;
    
    QFrame* sep1 = new QFrame();
    sep1->setFrameShape(QFrame::HLine);
    sep1->setStyleSheet("color: #333333;");
    grid->addWidget(sep1, row, 0, 1, 3);
    row++;
    
    // Leading (line spacing)
    QLabel* leadingLabel = new QLabel("Leading:");
    leadingLabel->setStyleSheet("color: #888888;");
    grid->addWidget(leadingLabel, row, 0);
    
    m_leadingSpinner = new QDoubleSpinBox();
    m_leadingSpinner->setRange(0.0, 1296.0);
    m_leadingSpinner->setValue(0.0);
    m_leadingSpinner->setDecimals(1);
    m_leadingSpinner->setSpecialValueText("Auto");
    m_leadingSpinner->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_leadingSpinner->setStyleSheet(kSpinStyle);
    connect(m_leadingSpinner, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
            this, &CharacterPanel::leadingChanged);
    grid->addWidget(m_leadingSpinner, row, 1);
    row++;
    
    // Tracking (letter spacing)
    QLabel* trackingLabel = new QLabel("Tracking:");
    trackingLabel->setStyleSheet("color: #888888;");
    grid->addWidget(trackingLabel, row, 0);
    
    m_trackingSpinner = new QDoubleSpinBox();
    m_trackingSpinner->setRange(-1000.0, 1000.0);
    m_trackingSpinner->setValue(0.0);
    m_trackingSpinner->setDecimals(0);
    m_trackingSpinner->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_trackingSpinner->setStyleSheet(kSpinStyle);
    connect(m_trackingSpinner, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
            this, &CharacterPanel::trackingChanged);
    grid->addWidget(m_trackingSpinner, row, 1);
    row++;
    
    QFrame* sep2 = new QFrame();
    sep2->setFrameShape(QFrame::HLine);
    sep2->setStyleSheet("color: #333333;");
    grid->addWidget(sep2, row, 0, 1, 3);
    row++;
    
    // Fill color
    QLabel* fillLabel = new QLabel("Fill:");
    fillLabel->setStyleSheet("color: #888888;");
    grid->addWidget(fillLabel, row, 0);
    
    m_fillColorBtn = new QPushButton();
    m_fillColorBtn->setFixedSize(32, 18);
    m_fillColorBtn->setStyleSheet(QString(kColorBtnStyle).arg(""));
    m_fillColorBtn->setStyleSheet(
        QString("QPushButton { background: #ffffff; border: 1px solid #555555; border-radius: 2px; }"
                "QPushButton:hover { border: 1px solid #ffffff; }"));
    connect(m_fillColorBtn, &QPushButton::clicked, this, [this]() {
        QColor color = QColorDialog::getColor(Qt::white, this, "Fill Color");
        if (color.isValid()) {
            m_fillColorBtn->setStyleSheet(
                QString("QPushButton { background: %1; border: 1px solid #555555; border-radius: 2px; }"
                        "QPushButton:hover { border: 1px solid #ffffff; }").arg(color.name()));
            emit fillColorChanged(color);
        }
    });
    grid->addWidget(m_fillColorBtn, row, 1);
    
    m_fillNoneBtn = new QPushButton("None");
    m_fillNoneBtn->setStyleSheet(kSmallBtnStyle);
    connect(m_fillNoneBtn, &QPushButton::clicked, this, [this]() {
        m_fillColorBtn->setStyleSheet(
            "QPushButton { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
            "stop:0 transparent, stop:0.45 transparent, stop:0.45 #cc0000, stop:0.55 #cc0000, stop:0.55 transparent, stop:1 transparent); "
            "border: 1px solid #555555; border-radius: 2px; }"
            "QPushButton:hover { border: 1px solid #ffffff; }");
        emit fillColorChanged(QColor(0, 0, 0, 0));
    });
    grid->addWidget(m_fillNoneBtn, row, 2);
    row++;
    
    // Stroke color
    QLabel* strokeLabel = new QLabel("Stroke:");
    strokeLabel->setStyleSheet("color: #888888;");
    grid->addWidget(strokeLabel, row, 0);
    
    m_strokeColorBtn = new QPushButton();
    m_strokeColorBtn->setFixedSize(32, 18);
    m_strokeColorBtn->setStyleSheet(
        "QPushButton { background: transparent; border: 1px solid #555555; border-radius: 2px; }"
        "QPushButton:hover { border: 1px solid #ffffff; }");
    connect(m_strokeColorBtn, &QPushButton::clicked, this, [this]() {
        QColor color = QColorDialog::getColor(Qt::black, this, "Stroke Color");
        if (color.isValid()) {
            m_strokeColorBtn->setStyleSheet(
                QString("QPushButton { background: %1; border: 1px solid #555555; border-radius: 2px; }"
                        "QPushButton:hover { border: 1px solid #ffffff; }").arg(color.name()));
            emit strokeColorChanged(color);
        }
    });
    grid->addWidget(m_strokeColorBtn, row, 1);
    
    m_strokeNoneBtn = new QPushButton("None");
    m_strokeNoneBtn->setStyleSheet(kSmallBtnStyle);
    connect(m_strokeNoneBtn, &QPushButton::clicked, this, [this]() {
        m_strokeColorBtn->setStyleSheet(
            "QPushButton { background: transparent; border: 1px solid #555555; border-radius: 2px; }"
            "QPushButton:hover { border: 1px solid #ffffff; }");
        emit strokeColorChanged(QColor(0, 0, 0, 0));
    });
    grid->addWidget(m_strokeNoneBtn, row, 2);
    row++;
    
    // Stroke width
    QLabel* strokeWidthLabel = new QLabel("Stroke W:");
    strokeWidthLabel->setStyleSheet("color: #888888;");
    grid->addWidget(strokeWidthLabel, row, 0);
    
    m_strokeWidthSpinner = new QDoubleSpinBox();
    m_strokeWidthSpinner->setRange(0.0, 100.0);
    m_strokeWidthSpinner->setValue(0.0);
    m_strokeWidthSpinner->setDecimals(1);
    m_strokeWidthSpinner->setSingleStep(1.0);
    m_strokeWidthSpinner->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_strokeWidthSpinner->setStyleSheet(kSpinStyle);
    connect(m_strokeWidthSpinner, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
            this, &CharacterPanel::strokeWidthChanged);
    grid->addWidget(m_strokeWidthSpinner, row, 1);
    row++;
    
    // Swap fill/stroke button
    m_swapFillStrokeBtn = new QPushButton("Swap Fill/Stroke");
    m_swapFillStrokeBtn->setStyleSheet(kSmallBtnStyle);
    connect(m_swapFillStrokeBtn, &QPushButton::clicked, this, [this]() {
        QString fillStyle = m_fillColorBtn->styleSheet();
        QString strokeStyle = m_strokeColorBtn->styleSheet();
        m_fillColorBtn->setStyleSheet(strokeStyle);
        m_strokeColorBtn->setStyleSheet(fillStyle);
    });
    grid->addWidget(m_swapFillStrokeBtn, row, 0, 1, 3);
    row++;
    
    QFrame* sep3 = new QFrame();
    sep3->setFrameShape(QFrame::HLine);
    sep3->setStyleSheet("color: #333333;");
    grid->addWidget(sep3, row, 0, 1, 3);
    row++;
    
    // Horizontal scale
    QLabel* hScaleLabel = new QLabel("H Scale:");
    hScaleLabel->setStyleSheet("color: #888888;");
    grid->addWidget(hScaleLabel, row, 0);
    
    m_hScaleSpinner = new QDoubleSpinBox();
    m_hScaleSpinner->setRange(0.0, 1000.0);
    m_hScaleSpinner->setValue(100.0);
    m_hScaleSpinner->setDecimals(1);
    m_hScaleSpinner->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_hScaleSpinner->setStyleSheet(kSpinStyle);
    connect(m_hScaleSpinner, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
            this, &CharacterPanel::horizontalScaleChanged);
    grid->addWidget(m_hScaleSpinner, row, 1);
    row++;
    
    // Vertical scale
    QLabel* vScaleLabel = new QLabel("V Scale:");
    vScaleLabel->setStyleSheet("color: #888888;");
    grid->addWidget(vScaleLabel, row, 0);
    
    m_vScaleSpinner = new QDoubleSpinBox();
    m_vScaleSpinner->setRange(0.0, 1000.0);
    m_vScaleSpinner->setValue(100.0);
    m_vScaleSpinner->setDecimals(1);
    m_vScaleSpinner->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_vScaleSpinner->setStyleSheet(kSpinStyle);
    connect(m_vScaleSpinner, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
            this, &CharacterPanel::verticalScaleChanged);
    grid->addWidget(m_vScaleSpinner, row, 1);
    row++;
    
    // Baseline shift
    QLabel* baseLabel = new QLabel("Baseline:");
    baseLabel->setStyleSheet("color: #888888;");
    grid->addWidget(baseLabel, row, 0);
    
    m_baselineShiftSpinner = new QDoubleSpinBox();
    m_baselineShiftSpinner->setRange(-1000.0, 1000.0);
    m_baselineShiftSpinner->setValue(0.0);
    m_baselineShiftSpinner->setDecimals(1);
    m_baselineShiftSpinner->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_baselineShiftSpinner->setStyleSheet(kSpinStyle);
    connect(m_baselineShiftSpinner, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
            this, &CharacterPanel::baselineShiftChanged);
    grid->addWidget(m_baselineShiftSpinner, row, 1);
    row++;
    
    QFrame* sep4 = new QFrame();
    sep4->setFrameShape(QFrame::HLine);
    sep4->setStyleSheet("color: #333333;");
    grid->addWidget(sep4, row, 0, 1, 3);
    row++;
    
    // Faux bold & italic
    m_fauxBoldCheck = new QCheckBox("Faux Bold");
    m_fauxBoldCheck->setStyleSheet("color: #cccccc;");
    connect(m_fauxBoldCheck, &QCheckBox::toggled, this, &CharacterPanel::fauxBoldToggled);
    grid->addWidget(m_fauxBoldCheck, row, 0, 1, 2);
    row++;
    
    m_fauxItalicCheck = new QCheckBox("Faux Italic");
    m_fauxItalicCheck->setStyleSheet("color: #cccccc;");
    connect(m_fauxItalicCheck, &QCheckBox::toggled, this, &CharacterPanel::fauxItalicToggled);
    grid->addWidget(m_fauxItalicCheck, row, 0, 1, 2);
    
    mainLayout->addLayout(grid);
    mainLayout->addStretch();
}

void CharacterPanel::setFont(const QFont& font) {
    m_fontCombo->setCurrentFont(font);
}

void CharacterPanel::setFontSize(double size) {
    m_sizeSpinner->setValue(size);
}

void CharacterPanel::setFillColor(const QColor& color) {
    if (color.alpha() == 0) {
        m_fillColorBtn->setStyleSheet(
            "QPushButton { background: transparent; border: 1px solid #555555; border-radius: 2px; }"
            "QPushButton:hover { border: 1px solid #ffffff; }");
    } else {
        m_fillColorBtn->setStyleSheet(
            QString("QPushButton { background: %1; border: 1px solid #555555; border-radius: 2px; }"
                    "QPushButton:hover { border: 1px solid #ffffff; }").arg(color.name()));
    }
}

void CharacterPanel::setStrokeColor(const QColor& color) {
    if (color.alpha() == 0) {
        m_strokeColorBtn->setStyleSheet(
            "QPushButton { background: transparent; border: 1px solid #555555; border-radius: 2px; }"
            "QPushButton:hover { border: 1px solid #ffffff; }");
    } else {
        m_strokeColorBtn->setStyleSheet(
            QString("QPushButton { background: %1; border: 1px solid #555555; border-radius: 2px; }"
                    "QPushButton:hover { border: 1px solid #ffffff; }").arg(color.name()));
    }
}

void CharacterPanel::setStrokeWidth(double width) {
    m_strokeWidthSpinner->setValue(width);
}

void CharacterPanel::setFauxBold(bool bold) {
    m_fauxBoldCheck->setChecked(bold);
}

void CharacterPanel::setFauxItalic(bool italic) {
    m_fauxItalicCheck->setChecked(italic);
}

} // namespace FreeEffect
