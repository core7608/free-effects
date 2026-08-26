#pragma once
#include <QWidget>
#include <QPushButton>
#include <QComboBox>
#include <QSlider>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>

namespace FreeEffect {

class PreviewPanel : public QWidget {
    Q_OBJECT
public:
    explicit PreviewPanel(QWidget* parent = nullptr);
    
    void setPlaying(bool playing);
    bool isPlaying() const { return m_playing; }
    void setCurrentFrame(int frame);
    void setTotalFrames(int total);
    void setResolution(double resolution);
    void setFPS(double fps);

signals:
    void playClicked();
    void pauseClicked();
    void stopClicked();
    void firstFrameClicked();
    void lastFrameClicked();
    void prevFrameClicked();
    void nextFrameClicked();
    void loopToggled(bool loop);
    void resolutionChanged(double resolution);
    void frameChanged(int frame);

private:
    QPushButton* m_firstFrameBtn;
    QPushButton* m_prevFrameBtn;
    QPushButton* m_playBtn;
    QPushButton* m_nextFrameBtn;
    QPushButton* m_lastFrameBtn;
    QPushButton* m_stopBtn;
    QPushButton* m_loopBtn;
    QComboBox* m_resolutionCombo;
    QLabel* m_frameLabel;
    QSlider* m_timelineSlider;
    bool m_playing = false;
    bool m_looping = true;
};

} // namespace FreeEffect
