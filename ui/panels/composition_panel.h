#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QTimer>
#include "../../core/timeline/composition.h"

namespace FreeEffect {

class CanvasWidget;
class MainWindow;

class CompositionPanel : public QWidget {
    Q_OBJECT
public:
    explicit CompositionPanel(MainWindow* parent);
    ~CompositionPanel() override = default;
    
    void setComposition(std::shared_ptr<Composition> comp);
    std::shared_ptr<Composition> getComposition() const { return m_composition; }
    CanvasWidget* getCanvas() const { return m_canvas; }
    
    void refreshView();
    void updateZoomCombo(double zoom);

signals:
    void timeChanged(double timeInSeconds);

private slots:
    void onZoomChanged(int index);
    void onResolutionChanged(int index);
    void onPlayPause();
    void onFitToWindow();

private:
    void setupUi();
    void setupToolbar();
    void setupBottomBar();
    void updateTimeDisplay();
    
    MainWindow* m_mainWindow;
    std::shared_ptr<Composition> m_composition;
    
    CanvasWidget* m_canvas;
    QVBoxLayout* m_mainLayout = nullptr;
    QComboBox* m_zoomCombo;
    QComboBox* m_resolutionCombo;
    QLabel* m_timeLabel;
    QPushButton* m_playButton;
    QPushButton* m_gridBtn = nullptr;
    QPushButton* m_rulersBtn = nullptr;
    QTimer* m_playTimer = nullptr;
    double m_currentTime = 0.0;
    bool m_playing = false;
};

} // namespace FreeEffect
