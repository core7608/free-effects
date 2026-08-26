#pragma once

#include <QWidget>
#include <QPushButton>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QRadioButton>
#include <QComboBox>

namespace FreeEffect {

class MainWindow;

class RotoBrushPanel : public QWidget {
    Q_OBJECT
public:
    explicit RotoBrushPanel(MainWindow* parent);
    ~RotoBrushPanel() override = default;

signals:
    void propagateForwardRequested();
    void propagateBackwardRequested();
    void freezeLayerRequested();
    void unfreezeLayerRequested();
    void refineEdgeRequested();
    void toggleForegroundMode();
    void toggleBackgroundMode();

private slots:
    void onPropagateForward();
    void onPropagateBackward();
    void onFreeze();
    void onUnfreeze();
    void onRefineEdge();
    void onBrushSizeChanged(int value);
    void onFeatherChanged(int value);
    void onContrastChanged(int value);

private:
    void setupUi();

    MainWindow* m_mainWindow;
    QSlider* m_brushSizeSlider;
    QSlider* m_featherSlider;
    QSlider* m_contrastSlider;
    QLabel* m_brushSizeLabel;
    QLabel* m_featherLabel;
    QLabel* m_contrastLabel;
    QRadioButton* m_foregroundRadio;
    QRadioButton* m_backgroundRadio;
    QPushButton* m_propagateForwardBtn;
    QPushButton* m_propagateBackwardBtn;
    QPushButton* m_refineEdgeBtn;
    QPushButton* m_freezeBtn;
    QPushButton* m_unfreezeBtn;
    QProgressBar* m_progressBar;
    QComboBox* m_edgeDetectionCombo;
};

} // namespace FreeEffect
