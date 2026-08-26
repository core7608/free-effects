#include "audio_panel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QStyle>

namespace FreeEffect {

AudioPanel::AudioPanel(QWidget* parent)
    : QWidget(parent) {
    
    setStyleSheet("background-color: #2a2a2a; color: #cccccc; font-size: 11px;");
    
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(8);
    
    // Audio header
    QLabel* header = new QLabel("Audio");
    header->setStyleSheet("color: #cccccc; font-size: 11px; font-weight: bold;");
    mainLayout->addWidget(header);
    
    // Volume control row
    QWidget* volumeRow = new QWidget();
    QHBoxLayout* volumeLayout = new QHBoxLayout(volumeRow);
    volumeLayout->setContentsMargins(0, 0, 0, 0);
    volumeLayout->setSpacing(6);
    
    QLabel* volNameLabel = new QLabel("Volume:");
    volNameLabel->setFixedWidth(50);
    volNameLabel->setStyleSheet("color: #888888;");
    volumeLayout->addWidget(volNameLabel);
    
    m_volumeSlider = new QSlider(Qt::Horizontal);
    m_volumeSlider->setRange(0, 100);
    m_volumeSlider->setValue(static_cast<int>(m_volume * 100));
    m_volumeSlider->setStyleSheet(
        "QSlider::groove:horizontal { background: #444444; height: 4px; border-radius: 2px; }"
        "QSlider::handle:horizontal { background: #00d4ff; width: 12px; margin: -4px 0; border-radius: 6px; }"
        "QSlider::sub-page:horizontal { background: #00d4ff; border-radius: 2px; }"
    );
    connect(m_volumeSlider, &QSlider::valueChanged, this, [this](int value) {
        m_volume = value / 100.0;
        m_volumeLabel->setText(QString("%1%").arg(value));
        emit volumeChanged(m_volume);
    });
    volumeLayout->addWidget(m_volumeSlider, 1);
    
    m_volumeLabel = new QLabel("75%");
    m_volumeLabel->setFixedWidth(36);
    m_volumeLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_volumeLabel->setStyleSheet("color: #cccccc;");
    volumeLayout->addWidget(m_volumeLabel);
    
    // Mute button
    m_muteButton = new QPushButton("M");
    m_muteButton->setFixedSize(28, 20);
    m_muteButton->setCheckable(true);
    m_muteButton->setToolTip("Mute Audio");
    m_muteButton->setStyleSheet(
        "QPushButton { background: #3c3c3c; color: #cccccc; border: 1px solid #555555; "
        "border-radius: 3px; font-size: 10px; font-weight: bold; }"
        "QPushButton:hover { background: #4a4a4a; }"
        "QPushButton:checked { background: #c8a000; color: #000000; border: 1px solid #c8a000; }"
    );
    connect(m_muteButton, &QPushButton::toggled, this, [this](bool checked) {
        m_muted = checked;
        m_muteButton->setText(checked ? "S" : "M");
        m_muteButton->setToolTip(checked ? "Unmute Audio" : "Mute Audio");
        emit muteToggled(m_muted);
    });
    volumeLayout->addWidget(m_muteButton);
    
    mainLayout->addWidget(volumeRow);
    
    // Level meters
    QLabel* meterHeader = new QLabel("Level Meters");
    meterHeader->setStyleSheet("color: #888888; font-size: 10px;");
    mainLayout->addWidget(meterHeader);
    
    QGridLayout* meterGrid = new QGridLayout();
    meterGrid->setSpacing(4);
    
    QLabel* leftLabel = new QLabel("L");
    leftLabel->setFixedWidth(14);
    leftLabel->setStyleSheet("color: #888888;");
    m_leftMeter = new QProgressBar();
    m_leftMeter->setRange(0, 100);
    m_leftMeter->setValue(0);
    m_leftMeter->setTextVisible(false);
    m_leftMeter->setFixedHeight(12);
    m_leftMeter->setStyleSheet(
        "QProgressBar { background: #1a1a1a; border: 1px solid #333333; border-radius: 2px; }"
        "QProgressBar::chunk { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "stop:0 #00cc00, stop:0.7 #cccc00, stop:1.0 #cc0000); border-radius: 2px; }"
    );
    meterGrid->addWidget(leftLabel, 0, 0);
    meterGrid->addWidget(m_leftMeter, 0, 1);
    
    QLabel* rightLabel = new QLabel("R");
    rightLabel->setFixedWidth(14);
    rightLabel->setStyleSheet("color: #888888;");
    m_rightMeter = new QProgressBar();
    m_rightMeter->setRange(0, 100);
    m_rightMeter->setValue(0);
    m_rightMeter->setTextVisible(false);
    m_rightMeter->setFixedHeight(12);
    m_rightMeter->setStyleSheet(m_leftMeter->styleSheet());
    meterGrid->addWidget(rightLabel, 1, 0);
    meterGrid->addWidget(m_rightMeter, 1, 1);
    
    mainLayout->addLayout(meterGrid);
    
    mainLayout->addStretch();
}

void AudioPanel::setVolume(double volume) {
    m_volume = qBound(0.0, volume, 1.0);
    m_volumeSlider->setValue(static_cast<int>(m_volume * 100));
}

void AudioPanel::setMuted(bool muted) {
    m_muted = muted;
    m_muteButton->setChecked(muted);
}

void AudioPanel::updateLevelMeter(float leftLevel, float rightLevel) {
    int leftVal = qBound(0.0f, leftLevel, 1.0f) * 100.0f;
    int rightVal = qBound(0.0f, rightLevel, 1.0f) * 100.0f;
    m_leftMeter->setValue(leftVal);
    m_rightMeter->setValue(rightVal);
}

} // namespace FreeEffect
