#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QTreeWidget>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include "../../core/timeline/layer.h"
#include "../../core/commands/command_stack.h"

namespace FreeEffect {

class MainWindow;

class EffectControlsPanel : public QWidget {
    Q_OBJECT
public:
    explicit EffectControlsPanel(MainWindow* parent);
    ~EffectControlsPanel() override = default;
    
    void setLayer(std::shared_ptr<Layer> layer);
    void setCommandStack(CommandStack* stack) { m_commandStack = stack; }
    void refreshControls();

signals:
    void propertyChanged();

private:
    void setupUi();
    void showEmptyState();
    void showLayerProperties();
    
    QWidget* createTransformRow(const QString& name, double value, const QString& suffix,
                                std::function<void(double)> setter,
                                bool hasKeyframe = false, double minVal = -99999, double maxVal = 99999);
    
    MainWindow* m_mainWindow;
    std::shared_ptr<Layer> m_layer;
    CommandStack* m_commandStack = nullptr;
    QLabel* m_emptyLabel;
    QWidget* m_propsWidget = nullptr;
    QVBoxLayout* m_propsLayout = nullptr;
};

} // namespace FreeEffect
