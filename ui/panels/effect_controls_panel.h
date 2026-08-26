#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QTreeWidget>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QFrame>
#include <QComboBox>
#include <QCheckBox>
#include <QLineEdit>
#include <vector>
#include <memory>
#include "../../core/timeline/layer.h"
#include "../../core/commands/command_stack.h"

namespace FreeEffect {

class Effect;
struct EffectParameter;
class MainWindow;

class EffectControlsPanel : public QWidget {
    Q_OBJECT
public:
    explicit EffectControlsPanel(MainWindow* parent);
    ~EffectControlsPanel() override = default;
    
    void setLayer(std::shared_ptr<Layer> layer);
    void setCommandStack(CommandStack* stack) { m_commandStack = stack; }
    void refreshControls();
    void addEffectToSelectedLayer(const QString& effectName);
    void removeEffectFromLayer(int effectIndex);

signals:
    void propertyChanged();
    void effectAdded();
    void effectRemoved();

private:
    void setupUi();
    void showEmptyState();
    void showLayerProperties();
    void showAppliedEffects();
    void showAddEffectMenu();
    QWidget* createEffectRow(int effectIndex, const std::shared_ptr<Effect>& effect);
    QWidget* createEffectParameterRow(const EffectParameter& param, const std::shared_ptr<Effect>& effect);
    QWidget* createTransformRow(const QString& name, double value, const QString& suffix,
                                std::function<void(double)> setter,
                                bool hasKeyframe = false, double minVal = -99999, double maxVal = 99999);
    
    MainWindow* m_mainWindow;
    std::shared_ptr<Layer> m_layer;
    CommandStack* m_commandStack = nullptr;
    QLabel* m_emptyLabel;
    QWidget* m_propsWidget = nullptr;
    QVBoxLayout* m_propsLayout = nullptr;
    QScrollArea* m_scrollArea = nullptr;
};

} // namespace FreeEffect
