#include "effect_controls_panel.h"
#include "../mainwindow/main_window.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QTreeWidgetItem>

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
    
    m_propsTree = new QTreeWidget(this);
    m_propsTree->setHeaderHidden(true);
    m_propsTree->setRootIsDecorated(true);
    m_propsTree->setIndentation(16);
    m_propsTree->setAnimated(true);
    m_propsTree->setStyleSheet(
        "QTreeWidget { background-color: #1a1a1a; color: #cccccc; border: none; font-size: 11px; }"
        "QTreeWidget::item { padding: 2px 4px; border: none; height: 22px; }"
        "QTreeWidget::item:selected { background-color: #ffffff; color: #000000; }"
        "QTreeWidget::item:hover { background-color: #2a2a2a; }"
    );
    m_propsTree->setVisible(false);
    layout->addWidget(m_propsTree, 1);
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
    m_propsTree->setVisible(false);
}

void EffectControlsPanel::showLayerProperties() {
    m_emptyLabel->setVisible(false);
    m_propsTree->setVisible(true);
    m_propsTree->clear();
    
    if (!m_layer) return;
    
    QTreeWidgetItem* transformGroup = createPropertyGroup("Transform", true);
    m_propsTree->addTopLevelItem(transformGroup);
    
    createPropertyItem(transformGroup, "Anchor Point", 
        QString::number(m_layer->getAnchorPoint().getDefaultValue(), 'f', 1));
    createPropertyItem(transformGroup, "Position", 
        QString::number(m_layer->getPosition().getDefaultValue(), 'f', 1));
    createPropertyItem(transformGroup, "Scale", 
        QString::number(m_layer->getScale().getDefaultValue(), 'f', 1) + "%");
    createPropertyItem(transformGroup, "Rotation", 
        QString::number(m_layer->getRotation().getDefaultValue(), 'f', 1) + QString::fromUtf8("\xc2\xb0"));
    createPropertyItem(transformGroup, "Opacity", 
        QString::number(m_layer->getOpacity().getDefaultValue() * 100.0, 'f', 0) + "%");
    
    transformGroup->setExpanded(true);
}

QTreeWidgetItem* EffectControlsPanel::createPropertyGroup(const QString& name, bool expanded) {
    QTreeWidgetItem* group = new QTreeWidgetItem();
    group->setText(0, "> " + name);
    group->setExpanded(expanded);
    group->setForeground(0, QColor(180, 180, 180));
    QFont font = group->font(0);
    font.setBold(true);
    font.setPointSize(11);
    group->setFont(0, font);
    return group;
}

QTreeWidgetItem* EffectControlsPanel::createPropertyItem(QTreeWidgetItem* parent, const QString& name, const QString& value) {
    QTreeWidgetItem* item = new QTreeWidgetItem(parent);
    item->setText(0, "    " + name + "    " + value);
    item->setForeground(0, QColor(170, 170, 170));
    return item;
}

void EffectControlsPanel::refreshControls() {
    if (m_layer) showLayerProperties();
}

} // namespace FreeEffect
