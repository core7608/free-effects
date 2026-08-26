#pragma once
#include <QWidget>
#include <QPushButton>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QLabel>

namespace FreeEffect {

class PaintPanel : public QWidget {
    Q_OBJECT
public:
    explicit PaintPanel(QWidget* parent = nullptr);

signals:
    void paintModeChanged(int mode);
    void strokeModeChanged(int mode);

private:
    QComboBox* m_toolCombo;
    QComboBox* m_modeCombo;
    QDoubleSpinBox* m_opacitySpinner;
    QDoubleSpinBox* m_flowSpinner;
    QPushButton* m_colorBtn;
    QComboBox* m_channelCombo;
};

} // namespace FreeEffect
