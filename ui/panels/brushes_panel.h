#pragma once
#include <QWidget>
#include <QListWidget>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include <vector>

namespace FreeEffect {

struct BrushPreset {
    QString name;
    double size = 10.0;
    double hardness = 100.0;
    double opacity = 100.0;
    double flow = 100.0;
    int spacing = 25;
};

class BrushesPanel : public QWidget {
    Q_OBJECT
public:
    explicit BrushesPanel(QWidget* parent = nullptr);
    
    void loadDefaultBrushes();
    BrushPreset getCurrentBrush() const;

signals:
    void brushChanged(const BrushPreset& brush);

private:
    QListWidget* m_brushList;
    QDoubleSpinBox* m_sizeSpinner;
    QDoubleSpinBox* m_hardnessSpinner;
    QDoubleSpinBox* m_opacitySpinner;
    QDoubleSpinBox* m_flowSpinner;
    QSpinBox* m_spacingSpinner;
    std::vector<BrushPreset> m_brushes;
};

} // namespace FreeEffect
