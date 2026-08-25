#pragma once

#include <QWidget>
#include <QTreeWidget>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QSplitter>
#include "../../core/timeline/composition.h"
#include "../../core/commands/command_stack.h"

namespace FreeEffect {

class MainWindow;
class TimelineRuler;
class KeyframeCanvas;

class TimelinePanel : public QWidget {
    Q_OBJECT
public:
    explicit TimelinePanel(MainWindow* parent);
    ~TimelinePanel() override = default;
    
    void setComposition(std::shared_ptr<Composition> comp, CommandStack* stack);
    void refreshTimeline();
    void setCurrentTime(double timeInSeconds);
    double getCurrentTime() const { return m_currentTime; }

signals:
    void currentTimeChanged(double timeInSeconds);
    void layerSelected(int index);

private slots:
    void onPlayPause();
    void onGoToStart();
    void onGoToEnd();
    void onZoomIn();
    void onZoomOut();
    void onLayerClicked(int row, int column);

private:
    void setupUi();
    void setupLayerList();
    void setupTimelineView();
    void setupTransportControls();
    void refreshLayerList();
    void updateTimeDisplay();
    
    MainWindow* m_mainWindow;
    std::shared_ptr<Composition> m_composition;
    CommandStack* m_commandStack = nullptr;
    double m_currentTime = 0.0;
    bool m_playing = false;
    
    QTreeWidget* m_layerTree;
    QWidget* m_timelineView;
    QLabel* m_timeLabel;
    QPushButton* m_playButton;
    QSlider* m_timeSlider;
    int m_selectedLayerIndex = -1;
};

} // namespace FreeEffect
