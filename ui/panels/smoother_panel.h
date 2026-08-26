#pragma once
#include <QWidget>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QSpinBox>

namespace FreeEffect {

class SmootherPanel : public QWidget {
    Q_OBJECT
public:
    explicit SmootherPanel(QWidget* parent = nullptr);

signals:
    void applySmoother(double spatialPrecision, double temporalPrecision);
    void applySimplifier(double tolerance, int maxKeyframes);

private:
    QDoubleSpinBox* m_spatialPrecisionSpinner;
    QDoubleSpinBox* m_temporalPrecisionSpinner;
    QPushButton* m_applySmoothBtn;
    QDoubleSpinBox* m_toleranceSpinner;
    QSpinBox* m_maxKeyframesSpinner;
    QPushButton* m_applySimplifyBtn;
};

} // namespace FreeEffect
