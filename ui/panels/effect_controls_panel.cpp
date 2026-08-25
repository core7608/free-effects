#include "effect_controls_panel.h"
#include "../mainwindow/main_window.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QScrollArea>
#include <QFrame>

namespace FreeEffect {

EffectControlsPanel::EffectControlsPanel(MainWindow* parent)
    : QWidget(parent)
    , m_mainWindow(parent) {
    setupUi();
}

void EffectControlsPanel::setupUi() {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    
    m_emptyLabel = new QLabel("No layer selected", this);
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_emptyLabel->setStyleSheet("color: #666666; font-size: 12px; background: transparent;");
    layout->addWidget(m_emptyLabel);
    
    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("QScrollArea { background: #1a1a1a; border: none; }");
    
    m_propsWidget = new QWidget();
    m_propsLayout = new QVBoxLayout(m_propsWidget);
    m_propsLayout->setContentsMargins(8, 8, 8, 8);
    m_propsLayout->setSpacing(4);
    
    scrollArea->setWidget(m_propsWidget);
    scrollArea->setVisible(false);
    layout->addWidget(scrollArea, 1);
}

void EffectControlsPanel::setLayer(std::shared_ptr<Layer> layer) {
    m_layer = layer;
    if (m_layer) {
        showLayerProperties();
    } else {
        showEmptyState();
    }
}

void EffectControlsPanel::showEmptyState() {
    m_emptyLabel->setVisible(true);
    if (m_propsWidget) m_propsWidget->parentWidget()->setVisible(false);
}

void EffectControlsPanel::showLayerProperties() {
    m_emptyLabel->setVisible(false);
    if (m_propsWidget) m_propsWidget->parentWidget()->setVisible(true);
    
    // Clear existing
    QLayoutItem* item;
    while ((item = m_propsLayout->takeAt(0)) != nullptr) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }
    
    if (!m_layer) return;
    
    // Layer name header
    QLabel* layerName = new QLabel(QString::fromStdString(m_layer->getName()));
    layerName->setStyleSheet("color: #ffffff; font-size: 13px; font-weight: bold; padding: 4px 0px;");
    m_propsLayout->addWidget(layerName);
    
    // Layer type
    QString typeStr;
    switch (m_layer->getType()) {
        case LayerType::Video: typeStr = "Video"; break;
        case LayerType::Audio: typeStr = "Audio"; break;
        case LayerType::Text: typeStr = "Text"; break;
        case LayerType::Shape: typeStr = "Shape"; break;
        case LayerType::Null: typeStr = "Null Object"; break;
        case LayerType::Solid: typeStr = "Solid"; break;
        default: typeStr = "Layer"; break;
    }
    QLabel* typeLabel = new QLabel(typeStr);
    typeLabel->setStyleSheet("color: #888888; font-size: 11px; padding: 0px 0px 8px 0px;");
    m_propsLayout->addWidget(typeLabel);
    
    // Separator
    QFrame* sep1 = new QFrame();
    sep1->setFrameShape(QFrame::HLine);
    sep1->setStyleSheet("color: #333333;");
    m_propsLayout->addWidget(sep1);
    
    // Transform group header
    QLabel* transformHeader = new QLabel("Transform");
    transformHeader->setStyleSheet("color: #cccccc; font-size: 11px; font-weight: bold; padding: 6px 0px 2px 0px;");
    m_propsLayout->addWidget(transformHeader);
    
    // Transform properties with editable spinboxes
    m_propsLayout->addWidget(createTransformRow("Anchor Point", 
        m_layer->getAnchorPoint().getDefaultValue(), "",
        [this](double v) { m_layer->getAnchorPoint().setDefaultValue(v); },
        m_layer->getAnchorPoint().hasKeyframes()));
    
    m_propsLayout->addWidget(createTransformRow("Position",
        m_layer->getPosition().getDefaultValue(), "",
        [this](double v) { m_layer->getPosition().setDefaultValue(v); },
        m_layer->getPosition().hasKeyframes()));
    
    m_propsLayout->addWidget(createTransformRow("Scale",
        m_layer->getScale().getDefaultValue(), "%",
        [this](double v) { m_layer->getScale().setDefaultValue(v); },
        m_layer->getScale().hasKeyframes(), 0.0, 1000.0));
    
    m_propsLayout->addWidget(createTransformRow("Rotation",
        m_layer->getRotation().getDefaultValue(), QString::fromUtf8("\xc2\xb0"),
        [this](double v) { m_layer->getRotation().setDefaultValue(v); },
        m_layer->getRotation().hasKeyframes()));
    
    m_propsLayout->addWidget(createTransformRow("Opacity",
        m_layer->getOpacity().getDefaultValue() * 100.0, "%",
        [this](double v) { m_layer->getOpacity().setDefaultValue(v / 100.0); },
        m_layer->getOpacity().hasKeyframes(), 0.0, 100.0));
    
    m_propsLayout->addStretch();
}

QWidget* EffectControlsPanel::createTransformRow(const QString& name, double value, const QString& suffix,
                                                  std::function<void(double)> setter,
                                                  bool hasKeyframe, double minVal, double maxVal) {
    QWidget* row = new QWidget();
    QHBoxLayout* hlayout = new QHBoxLayout(row);
    hlayout->setContentsMargins(0, 2, 0, 2);
    hlayout->setSpacing(4);
    
    // Keyframe diamond button
    QPushButton* kfBtn = new QPushButton();
    kfBtn->setFixedSize(16, 16);
    kfBtn->setToolTip("Toggle Keyframe");
    if (hasKeyframe) {
        kfBtn->setStyleSheet(
            "QPushButton { background: #c8a000; border: none; border-radius: 2px; }"
            "QPushButton:hover { background: #e0b800; }"
        );
    } else {
        kfBtn->setStyleSheet(
            "QPushButton { background: #444444; border: none; border-radius: 2px; }"
            "QPushButton:hover { background: #666666; }"
        );
    }
    
    // Property name
    QLabel* nameLabel = new QLabel(name);
    nameLabel->setFixedWidth(100);
    nameLabel->setStyleSheet("color: #aaaaaa; font-size: 11px; background: transparent;");
    
    // Value spinbox
    QDoubleSpinBox* spinbox = new QDoubleSpinBox();
    spinbox->setRange(minVal, maxVal);
    spinbox->setValue(value);
    spinbox->setDecimals(1);
    spinbox->setSingleStep(1.0);
    spinbox->setButtonSymbols(QAbstractSpinBox::NoButtons);
    spinbox->setFixedWidth(80);
    spinbox->setStyleSheet(
        "QDoubleSpinBox { background-color: #2a2a2a; color: #cccccc; border: 1px solid #444444; "
        "border-radius: 2px; padding: 2px 4px; font-size: 11px; }"
        "QDoubleSpinBox:focus { border: 1px solid #ffffff; }"
        "QDoubleSpinBox::up-button, QDoubleSpinBox::down-button { width: 0px; height: 0px; }"
    );
    connect(spinbox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [setter](double v) {
        setter(v);
    });
    
    hlayout->addWidget(kfBtn);
    hlayout->addWidget(nameLabel);
    hlayout->addWidget(spinbox);
    if (!suffix.isEmpty()) {
        QLabel* suffixLabel = new QLabel(suffix);
        suffixLabel->setStyleSheet("color: #888888; font-size: 11px; background: transparent;");
        hlayout->addWidget(suffixLabel);
    }
    hlayout->addStretch();
    
    return row;
}

void EffectControlsPanel::refreshControls() {
    if (m_layer) showLayerProperties();
}

} // namespace FreeEffect
