#pragma once
#include <QWidget>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QTimer>

namespace FreeEffect {

class AudioPanel : public QWidget {
    Q_OBJECT
public:
    explicit AudioPanel(QWidget* parent = nullptr);
    
    void setVolume(double volume);
    double getVolume() const { return m_volume; }
    void setMuted(bool muted);
    bool isMuted() const { return m_muted; }
    void updateLevelMeter(float leftLevel, float rightLevel);

signals:
    void volumeChanged(double volume);
    void muteToggled(bool muted);
    void audioScrubbed(double time);

private:
    QSlider* m_volumeSlider;
    QLabel* m_volumeLabel;
    QPushButton* m_muteButton;
    QProgressBar* m_leftMeter;
    QProgressBar* m_rightMeter;
    double m_volume = 0.75;
    bool m_muted = false;
};

} // namespace FreeEffect
