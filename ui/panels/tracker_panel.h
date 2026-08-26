#pragma once

#include <QWidget>
#include <QPushButton>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QCheckBox>

namespace FreeEffect {

class MainWindow;

class TrackerPanel : public QWidget {
    Q_OBJECT
public:
    explicit TrackerPanel(MainWindow* parent);
    ~TrackerPanel() override = default;

signals:
    void trackForwardRequested();
    void trackBackwardRequested();
    void stabilizeRequested();
    void cameraTrackRequested();
    void resetTrackPointRequested();

private slots:
    void onTrackForward();
    void onTrackBackward();
    void onStabilize();
    void onCameraTrack();
    void onResetTrack();
    void onTrackTypeChanged(int index);

private:
    void setupUi();

    MainWindow* m_mainWindow;
    QComboBox* m_trackTypeCombo;
    QComboBox* m_trackForCombo;
    QSpinBox* m_searchRegionSpin;
    QSpinBox* m_featureRegionSpin;
    QDoubleSpinBox* m_confidenceThresholdSpin;
    QSpinBox* m_framesBackSpin;
    QPushButton* m_trackForwardBtn;
    QPushButton* m_trackBackwardBtn;
    QPushButton* m_stabilizeBtn;
    QPushButton* m_cameraTrackBtn;
    QPushButton* m_resetBtn;
    QProgressBar* m_progressBar;
    QCheckBox* m_adaptiveFeatureCheck;
    QCheckBox* m_subpixelCheck;
};

} // namespace FreeEffect
