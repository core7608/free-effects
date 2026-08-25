#include "composition_panel.h"
#include "../canvas/canvas_widget.h"
#include "../mainwindow/main_window.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>

namespace FreeEffect {

CompositionPanel::CompositionPanel(MainWindow* parent)
    : QWidget(parent)
    , m_mainWindow(parent) {
    setupUi();
}

void CompositionPanel::setupUi() {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    
    setupToolbar();
    
    m_canvas = new CanvasWidget(this);
    layout->addWidget(m_canvas, 1);
    
    setupBottomBar();
}

void CompositionPanel::setupToolbar() {
    QHBoxLayout* toolbar = new QHBoxLayout();
    toolbar->setContentsMargins(4, 2, 4, 2);
    
    QLabel* zoomLabel = new QLabel("Zoom:", this);
    m_zoomCombo = new QComboBox(this);
    m_zoomCombo->addItems({"25%", "50%", "100%", "150%", "200%", "Fit"});
    m_zoomCombo->setCurrentText("Fit");
    connect(m_zoomCombo, QOverload<int>::of(&QComboBox::activated), this, &CompositionPanel::onZoomChanged);
    
    QLabel* resLabel = new QLabel("Resolution:", this);
    m_resolutionCombo = new QComboBox(this);
    m_resolutionCombo->addItems({"Full", "Half", "Third", "Quarter"});
    connect(m_resolutionCombo, QOverload<int>::of(&QComboBox::activated), this, &CompositionPanel::onResolutionChanged);
    
    QPushButton* gridBtn = new QPushButton("Grid", this);
    QPushButton* rulersBtn = new QPushButton("Rulers", this);
    QPushButton* guidesBtn = new QPushButton("Guides", this);
    
    toolbar->addWidget(zoomLabel);
    toolbar->addWidget(m_zoomCombo);
    toolbar->addSpacing(10);
    toolbar->addWidget(resLabel);
    toolbar->addWidget(m_resolutionCombo);
    toolbar->addSpacing(10);
    toolbar->addWidget(gridBtn);
    toolbar->addWidget(rulersBtn);
    toolbar->addWidget(guidesBtn);
    toolbar->addStretch();
    
    layout()->addItem(toolbar);
}

void CompositionPanel::setupBottomBar() {
    QHBoxLayout* bottomBar = new QHBoxLayout();
    bottomBar->setContentsMargins(4, 2, 4, 2);
    
    m_timeLabel = new QLabel("00:00:00:00", this);
    m_timeLabel->setFont(QFont("Monospace", 10));
    
    m_playButton = new QPushButton("Play", this);
    connect(m_playButton, &QPushButton::clicked, this, &CompositionPanel::onPlayClicked);
    
    QPushButton* loopBtn = new QPushButton("Loop", this);
    
    bottomBar->addWidget(m_timeLabel);
    bottomBar->addStretch();
    bottomBar->addWidget(m_playButton);
    bottomBar->addWidget(loopBtn);
    
    layout()->addItem(bottomBar);
}

void CompositionPanel::setComposition(std::shared_ptr<Composition> comp) {
    m_composition = comp;
    m_currentTime = 0.0;
    updateTimeDisplay();
    if (m_canvas) m_canvas->setComposition(comp);
}

void CompositionPanel::refreshView() {
    if (m_canvas) m_canvas->update();
}

void CompositionPanel::updateTimeDisplay() {
    if (!m_composition) {
        m_timeLabel->setText("00:00:00:00");
        return;
    }
    
    int totalFrames = static_cast<int>(m_currentTime * m_composition->getFrameRate().fps);
    int fps = static_cast<int>(m_composition->getFrameRate().fps);
    int frames = totalFrames % fps;
    int seconds = (totalFrames / fps) % 60;
    int minutes = (totalFrames / (fps * 60)) % 60;
    int hours = totalFrames / (fps * 3600);
    
    m_timeLabel->setText(QString("%1:%2:%3:%4")
        .arg(hours, 2, 10, QChar('0'))
        .arg(minutes, 2, 10, QChar('0'))
        .arg(seconds, 2, 10, QChar('0'))
        .arg(frames, 2, 10, QChar('0')));
}

void CompositionPanel::onZoomChanged(int index) {
    Q_UNUSED(index);
}

void CompositionPanel::onResolutionChanged(int index) {
    Q_UNUSED(index);
}

void CompositionPanel::onPlayClicked() {
    m_playing = true;
    m_playButton->setText("Pause");
    disconnect(m_playButton, &QPushButton::clicked, this, &CompositionPanel::onPlayClicked);
    connect(m_playButton, &QPushButton::clicked, this, &CompositionPanel::onPauseClicked);
}

void CompositionPanel::onPauseClicked() {
    m_playing = false;
    m_playButton->setText("Play");
    disconnect(m_playButton, &QPushButton::clicked, this, &CompositionPanel::onPauseClicked);
    connect(m_playButton, &QPushButton::clicked, this, &CompositionPanel::onPlayClicked);
}

} // namespace FreeEffect
