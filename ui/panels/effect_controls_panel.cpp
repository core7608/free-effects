#include "effect_controls_panel.h"
#include "../mainwindow/main_window.h"
#include "../../core/effects/effect_registry.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QScrollArea>
#include <QFrame>
#include <QComboBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QFileDialog>
#include <QColorDialog>
#include <QMenu>
#include <QAction>

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
    
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setStyleSheet("QScrollArea { background: #1a1a1a; border: none; }");
    
    m_propsWidget = new QWidget();
    m_propsLayout = new QVBoxLayout(m_propsWidget);
    m_propsLayout->setContentsMargins(8, 8, 8, 8);
    m_propsLayout->setSpacing(4);
    
    m_scrollArea->setWidget(m_propsWidget);
    m_scrollArea->setVisible(false);
    layout->addWidget(m_scrollArea, 1);
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
    if (m_scrollArea) m_scrollArea->setVisible(false);
}

void EffectControlsPanel::showLayerProperties() {
    m_emptyLabel->setVisible(false);
    if (m_scrollArea) m_scrollArea->setVisible(true);
    
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
    
    // Transform properties
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
    
    // Separator before effects
    QFrame* sep2 = new QFrame();
    sep2->setFrameShape(QFrame::HLine);
    sep2->setStyleSheet("color: #333333;");
    m_propsLayout->addWidget(sep2);
    
    // Applied Effects section
    showAppliedEffects();
    
    m_propsLayout->addStretch();
}

void EffectControlsPanel::showAppliedEffects() {
    if (!m_layer) return;
    
    // Effects header with Add button
    QWidget* effectsHeader = new QWidget();
    QHBoxLayout* headerLayout = new QHBoxLayout(effectsHeader);
    headerLayout->setContentsMargins(0, 4, 0, 4);
    headerLayout->setSpacing(8);
    
    QLabel* effectsLabel = new QLabel("Effects");
    effectsLabel->setStyleSheet("color: #cccccc; font-size: 11px; font-weight: bold;");
    headerLayout->addWidget(effectsLabel);
    
    QPushButton* addEffectBtn = new QPushButton("+");
    addEffectBtn->setFixedSize(20, 20);
    addEffectBtn->setToolTip("Add Effect");
    addEffectBtn->setStyleSheet(
        "QPushButton { background: #2a2a2a; color: #cccccc; border: 1px solid #555555; "
        "border-radius: 3px; font-size: 12px; font-weight: bold; }"
        "QPushButton:hover { background: #3a3a3a; border: 1px solid #00d4ff; color: #00d4ff; }"
    );
    connect(addEffectBtn, &QPushButton::clicked, this, &EffectControlsPanel::showAddEffectMenu);
    headerLayout->addWidget(addEffectBtn);
    headerLayout->addStretch();
    
    m_propsLayout->addWidget(effectsHeader);
    
    // Show each applied effect
    const auto& effects = m_layer->getEffects();
    for (int i = 0; i < static_cast<int>(effects.size()); ++i) {
        m_propsLayout->addWidget(createEffectRow(i, effects[i]));
    }
    
    if (effects.empty()) {
        QLabel* noEffects = new QLabel("  No effects applied");
        noEffects->setStyleSheet("color: #666666; font-size: 11px; padding: 4px 0px;");
        m_propsLayout->addWidget(noEffects);
    }
}

QWidget* EffectControlsPanel::createEffectRow(int effectIndex, const std::shared_ptr<Effect>& effect) {
    QWidget* effectWidget = new QWidget();
    QVBoxLayout* effectLayout = new QVBoxLayout(effectWidget);
    effectLayout->setContentsMargins(0, 2, 0, 4);
    effectLayout->setSpacing(2);
    
    // Effect header row
    QWidget* headerRow = new QWidget();
    QHBoxLayout* headerLayout = new QHBoxLayout(headerRow);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(4);
    
    // Enable/disable checkbox
    QCheckBox* enableCheck = new QCheckBox();
    enableCheck->setChecked(effect->isEnabled());
    enableCheck->setToolTip("Enable/Disable Effect");
    connect(enableCheck, &QCheckBox::toggled, this, [effect](bool checked) {
        effect->setEnabled(checked);
    });
    headerLayout->addWidget(enableCheck);
    
    // Effect name
    QLabel* nameLabel = new QLabel(QString::fromStdString(effect->getName()));
    nameLabel->setStyleSheet("color: #00d4ff; font-size: 11px; font-weight: bold; background: transparent;");
    headerLayout->addWidget(nameLabel);
    
    headerLayout->addStretch();
    
    // Remove button
    QPushButton* removeBtn = new QPushButton("x");
    removeBtn->setFixedSize(16, 16);
    removeBtn->setToolTip("Remove Effect");
    removeBtn->setStyleSheet(
        "QPushButton { background: transparent; color: #888888; border: none; font-size: 10px; }"
        "QPushButton:hover { color: #ff4444; }"
    );
    connect(removeBtn, &QPushButton::clicked, this, [this, effectIndex]() {
        removeEffectFromLayer(effectIndex);
    });
    headerLayout->addWidget(removeBtn);
    
    effectLayout->addWidget(headerRow);
    
    // Effect parameters
    const auto& params = effect->getParameters();
    for (const auto& param : params) {
        effectLayout->addWidget(createEffectParameterRow(param, effect));
    }
    
    // Separator
    QFrame* sep = new QFrame();
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("color: #2a2a2a;");
    effectLayout->addWidget(sep);
    
    return effectWidget;
}

QWidget* EffectControlsPanel::createEffectParameterRow(const EffectParameter& param, const std::shared_ptr<Effect>& effect) {
    QWidget* row = new QWidget();
    QHBoxLayout* hlayout = new QHBoxLayout(row);
    hlayout->setContentsMargins(8, 1, 0, 1);
    hlayout->setSpacing(4);
    
    // Parameter name
    QLabel* nameLabel = new QLabel(QString::fromStdString(param.label));
    nameLabel->setFixedWidth(100);
    nameLabel->setStyleSheet("color: #aaaaaa; font-size: 11px; background: transparent;");
    hlayout->addWidget(nameLabel);
    
    switch (param.type) {
        case ParameterType::Float: {
            QDoubleSpinBox* spinbox = new QDoubleSpinBox();
            spinbox->setRange(param.floatMin, param.floatMax);
            spinbox->setValue(param.floatValue);
            spinbox->setDecimals(2);
            spinbox->setSingleStep(0.1);
            spinbox->setButtonSymbols(QAbstractSpinBox::NoButtons);
            spinbox->setFixedWidth(80);
            spinbox->setStyleSheet(
                "QDoubleSpinBox { background-color: #2a2a2a; color: #cccccc; border: 1px solid #444444; "
                "border-radius: 2px; padding: 2px 4px; font-size: 11px; }"
                "QDoubleSpinBox:focus { border: 1px solid #ffffff; }"
                "QDoubleSpinBox::up-button, QDoubleSpinBox::down-button { width: 0px; height: 0px; }"
            );
            connect(spinbox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, 
                [effect, name = param.name](double v) {
                    effect->setParameterValue(name, v);
                });
            hlayout->addWidget(spinbox);
            break;
        }
        case ParameterType::Int: {
            QSpinBox* spinbox = new QSpinBox();
            spinbox->setRange(param.intMin, param.intMax);
            spinbox->setValue(param.intValue);
            spinbox->setButtonSymbols(QAbstractSpinBox::NoButtons);
            spinbox->setFixedWidth(80);
            spinbox->setStyleSheet(
                "QSpinBox { background-color: #2a2a2a; color: #cccccc; border: 1px solid #444444; "
                "border-radius: 2px; padding: 2px 4px; font-size: 11px; }"
                "QSpinBox:focus { border: 1px solid #ffffff; }"
                "QSpinBox::up-button, QSpinBox::down-button { width: 0px; height: 0px; }"
            );
            connect(spinbox, QOverload<int>::of(&QSpinBox::valueChanged), this, 
                [effect, name = param.name](int v) {
                    effect->setParameterValue(name, v);
                });
            hlayout->addWidget(spinbox);
            break;
        }
        case ParameterType::Bool: {
            QCheckBox* checkbox = new QCheckBox();
            checkbox->setChecked(param.boolValue);
            connect(checkbox, &QCheckBox::toggled, this, 
                [effect, name = param.name](bool v) {
                    effect->setParameterValue(name, v);
                });
            hlayout->addWidget(checkbox);
            break;
        }
        case ParameterType::Color: {
            QPushButton* colorBtn = new QPushButton();
            colorBtn->setFixedSize(40, 18);
            colorBtn->setToolTip("Click to choose color");
            QColor c(static_cast<int>(param.colorValue.r * 255),
                     static_cast<int>(param.colorValue.g * 255),
                     static_cast<int>(param.colorValue.b * 255));
            colorBtn->setStyleSheet(
                QString("QPushButton { background: %1; border: 1px solid #555555; border-radius: 2px; }"
                        "QPushButton:hover { border: 1px solid #ffffff; }").arg(c.name()));
            connect(colorBtn, &QPushButton::clicked, this, 
                [effect, name = param.name, colorBtn, this](bool) {
                    QColor initial(static_cast<int>(effect->getColorParam(name).r * 255),
                                   static_cast<int>(effect->getColorParam(name).g * 255),
                                   static_cast<int>(effect->getColorParam(name).b * 255));
                    QColor color = QColorDialog::getColor(initial, this, "Choose Color");
                    if (color.isValid()) {
                        Color c{color.redF(), color.greenF(), color.blueF(), 1.0};
                        effect->setParameterValue(name, c);
                        colorBtn->setStyleSheet(
                            QString("QPushButton { background: %1; border: 1px solid #555555; border-radius: 2px; }"
                                    "QPushButton:hover { border: 1px solid #ffffff; }").arg(color.name()));
                    }
                });
            hlayout->addWidget(colorBtn);
            break;
        }
        case ParameterType::Dropdown: {
            QComboBox* combo = new QComboBox();
            for (const auto& opt : param.dropdownOptions) {
                combo->addItem(QString::fromStdString(opt));
            }
            combo->setCurrentIndex(param.dropdownValue);
            combo->setFixedWidth(100);
            combo->setStyleSheet(
                "QComboBox { background-color: #2a2a2a; color: #cccccc; border: 1px solid #444444; "
                "border-radius: 2px; padding: 2px 4px; font-size: 11px; }"
                "QComboBox::drop-down { border: none; }"
                "QComboBox QAbstractItemView { background: #2a2a2a; color: #cccccc; selection-background-color: #2a5a7a; }"
            );
            connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, 
                [effect, name = param.name](int v) {
                    effect->setParameterValue(name, v);
                });
            hlayout->addWidget(combo);
            break;
        }
        case ParameterType::Angle: {
            QDoubleSpinBox* spinbox = new QDoubleSpinBox();
            spinbox->setRange(0.0, 360.0);
            spinbox->setValue(param.angleValue);
            spinbox->setDecimals(1);
            spinbox->setSuffix(QString::fromUtf8("\xc2\xb0"));
            spinbox->setSingleStep(1.0);
            spinbox->setButtonSymbols(QAbstractSpinBox::NoButtons);
            spinbox->setFixedWidth(80);
            spinbox->setStyleSheet(
                "QDoubleSpinBox { background-color: #2a2a2a; color: #cccccc; border: 1px solid #444444; "
                "border-radius: 2px; padding: 2px 4px; font-size: 11px; }"
                "QDoubleSpinBox:focus { border: 1px solid #ffffff; }"
                "QDoubleSpinBox::up-button, QDoubleSpinBox::down-button { width: 0px; height: 0px; }"
            );
            connect(spinbox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, 
                [effect, name = param.name](double v) {
                    effect->setParameterValue(name, v);
                });
            hlayout->addWidget(spinbox);
            break;
        }
        default:
            break;
    }
    
    hlayout->addStretch();
    return row;
}

void EffectControlsPanel::showAddEffectMenu() {
    if (!m_layer) return;
    
    QMenu menu(this);
    menu.setStyleSheet(
        "QMenu { background: #2a2a2a; color: #cccccc; border: 1px solid #555555; }"
        "QMenu::item { padding: 4px 20px; }"
        "QMenu::item:selected { background: #2a5a7a; }"
        "QMenu::separator { height: 1px; background: #555555; margin: 2px 8px; }"
    );
    
    auto& registry = EffectRegistry::instance();
    auto categories = registry.getCategories();
    
    for (const auto& cat : categories) {
        QMenu* catMenu = menu.addMenu(QString::fromStdString(cat));
        auto effects = registry.getEffectNamesInCategory(cat);
        for (const auto& effName : effects) {
            QString qName = QString::fromStdString(effName);
            QAction* action = catMenu->addAction(qName);
            connect(action, &QAction::triggered, this, [this, qName]() {
                addEffectToSelectedLayer(qName);
            });
        }
    }
    
    menu.exec(QCursor::pos());
}

void EffectControlsPanel::addEffectToSelectedLayer(const QString& effectName) {
    if (!m_layer) return;
    
    auto& registry = EffectRegistry::instance();
    if (!registry.hasEffect(effectName.toStdString())) return;
    
    auto effect = registry.create(effectName.toStdString());
    if (effect) {
        m_layer->addEffect(std::shared_ptr<Effect>(std::move(effect)));
        showLayerProperties();
        emit effectAdded();
    }
}

void EffectControlsPanel::removeEffectFromLayer(int effectIndex) {
    if (!m_layer) return;
    m_layer->removeEffect(effectIndex);
    showLayerProperties();
    emit effectRemoved();
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
