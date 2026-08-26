#pragma once

#include <QWidget>
#include <QPushButton>
#include <QComboBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QListWidget>

namespace FreeEffect {

class MainWindow;

class PlanarEditorPanel : public QWidget {
    Q_OBJECT
public:
    explicit PlanarEditorPanel(MainWindow* parent);
    ~PlanarEditorPanel() override = default;

signals:
    void trackForwardRequested();
    void trackBackwardRequested();
    void exportToAERequested();
    void adjustPointsRequested();
    void resetSurfaceRequested();

private slots:
    void onTrackForward();
    void onTrackBackward();
    void onExportToAE();
    void onAdjustPoints();
    void onResetSurface();
    void onSurfaceModeChanged(int index);

private:
    void setupUi();

    MainWindow* m_mainWindow;
    QComboBox* m_surfaceModeCombo;
    QComboBox* m_trackForCombo;
    QPushButton* m_trackForwardBtn;
    QPushButton* m_trackBackwardBtn;
    QPushButton* m_exportToAEBtn;
    QPushButton* m_adjustPointsBtn;
    QPushButton* m_resetSurfaceBtn;
    QProgressBar* m_progressBar;
    QListWidget* m_pointList;
};

} // namespace FreeEffect
