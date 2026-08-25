#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
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
    
    MainWindow* m_mainWindow;
    std::shared_ptr<Layer> m_layer;
    QLabel* m_emptyLabel;
    QWidget* m_propertiesWidget;
};

} // namespace FreeEffect
