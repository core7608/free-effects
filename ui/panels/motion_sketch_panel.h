#pragma once
#include <QWidget>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QSpinBox>

namespace FreeEffect {

class MotionSketchPanel : public QWidget {
    Q_OBJECT
public:
    explicit MotionSketchPanel(QWidget* parent = nullptr);

signals:
    void startCapture();
    void stopCapture();
    void applyToPosition();
    void createNewLayer();

private:
    QPushButton* m_startCaptureBtn;
    QPushButton* m_stopCaptureBtn;
    QPushButton* m_applyBtn;
    QPushButton* m_createLayerBtn;
    QDoubleSpinBox* m_captureRateSpinner;
    QCheckBox* m_includeRotationCheck;
    QCheckBox* m_includeScaleCheck;
    QLabel* m_statusLabel;
};

} // namespace FreeEffect
