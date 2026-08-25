#include "effect_controls_panel.h"
#include "../mainwindow/main_window.h"
#include <QVBoxLayout>
#include <QLabel>

namespace FreeEffect {

EffectControlsPanel::EffectControlsPanel(MainWindow* parent)
    : QWidget(parent)
    , m_mainWindow(parent) {
    setupUi();
}

void EffectControlsPanel::setupUi() {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);
    
    m_emptyLabel = new QLabel("No layer selected", this);
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_emptyLabel->setStyleSheet("color: #888; font-size: 12px;");
    layout->addWidget(m_emptyLabel);
    
    m_propertiesWidget = new QWidget(this);
    m_propertiesWidget->setVisible(false);
    layout->addWidget(m_propertiesWidget);
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
    m_propertiesWidget->setVisible(false);
}

void EffectControlsPanel::showLayerProperties() {
    m_emptyLabel->setVisible(false);
    m_propertiesWidget->setVisible(true);
    
    // Clear old properties
    QLayout* oldLayout = m_propertiesWidget->layout();
    if (oldLayout) {
        QLayoutItem* item;
        while ((item = oldLayout->takeAt(0)) != nullptr) {
            if (item->widget()) item->widget()->deleteLater();
            delete item;
        }
        delete oldLayout;
    }
    
    if (!m_layer) return;
    
    QVBoxLayout* layout = new QVBoxLayout(m_propertiesWidget);
    
    QLabel* nameLabel = new QLabel(QString("<b>%1</b>").arg(QString::fromStdString(m_layer->getName())), this);
    layout->addWidget(nameLabel);
    
    auto addPropertyRow = [&](const QString& name, const PropertyTrack& track) {
        QHBoxLayout* row = new QHBoxLayout();
        QLabel* propLabel = new QLabel(name, this);
        propLabel->setFixedWidth(100);
        QLabel* valueLabel = new QLabel(QString::number(track.getDefaultValue()), this);
        row->addWidget(propLabel);
        row->addWidget(valueLabel);
        row->addStretch();
        layout->addLayout(row);
    };
    
    addPropertyRow("Position", m_layer->getPosition());
    addPropertyRow("Scale", m_layer->getScale());
    addPropertyRow("Rotation", m_layer->getRotation());
    addPropertyRow("Opacity", m_layer->getOpacity());
    addPropertyRow("Anchor Point", m_layer->getAnchorPoint());
    
    layout->addStretch();
}

void EffectControlsPanel::refreshControls() {
    if (m_layer) showLayerProperties();
}

} // namespace FreeEffect
