#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QTreeWidget>
#include "../../core/timeline/layer.h"

namespace FreeEffect {

class MainWindow;

class EffectControlsPanel : public QWidget {
    Q_OBJECT
public:
    explicit EffectControlsPanel(MainWindow* parent);
    ~EffectControlsPanel() override = default;
    
    void setLayer(std::shared_ptr<Layer> layer);
    void refreshControls();

private:
    void setupUi();
    void showEmptyState();
    void showLayerProperties();
    QTreeWidgetItem* createPropertyGroup(const QString& name, bool expanded = true);
    QTreeWidgetItem* createPropertyItem(QTreeWidgetItem* parent, const QString& name, const QString& value);
    
    MainWindow* m_mainWindow;
    std::shared_ptr<Layer> m_layer;
    QLabel* m_emptyLabel;
    QTreeWidget* m_propsTree;
};

} // namespace FreeEffect
