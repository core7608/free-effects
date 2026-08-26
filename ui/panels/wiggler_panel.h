#pragma once
#include <QWidget>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include <QVBoxLayout>

namespace FreeEffect {

class WigglerPanel : public QWidget {
    Q_OBJECT
public:
    explicit WigglerPanel(QWidget* parent = nullptr);

signals:
    void applyWiggle(double frequency, double amplitude, int dimensions, bool affectPosition, bool affectRotation, bool affectScale);
    void applySmoothWiggle(double frequency, double amplitude);

private:
    QDoubleSpinBox* m_frequencySpinner;
    QDoubleSpinBox* m_amplitudeSpinner;
    QComboBox* m_dimensionsCombo;
    QCheckBox* m_positionCheck;
    QCheckBox* m_rotationCheck;
    QCheckBox* m_scaleCheck;
    QPushButton* m_applyBtn;
    QPushButton* m_smoothApplyBtn;
};

} // namespace FreeEffect
