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
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    
    setupToolbar();
    
    m_canvas = new CanvasWidget(this);
    m_mainLayout->addWidget(m_canvas, 1);
    
    setupBottomBar();
}

void CompositionPanel::setupToolbar() {
    QWidget* toolbarWidget = new QWidget(this);
    toolbarWidget->setFixedHeight(30);
    toolbarWidget->setStyleSheet("background-color: #333333; border-bottom: 1px solid #1a1a1a;");
    
    QHBoxLayout* toolbar = new QHBoxLayout(toolbarWidget);
    toolbar->setContentsMargins(6, 2, 6, 2);
    toolbar->setSpacing(6);
    
    QLabel* zoomLabel = new QLabel("Zoom:", this);
    zoomLabel->setStyleSheet("color: #888888; font-size: 11px;");
    m_zoomCombo = new QComboBox(this);
    m_zoomCombo->addItems({"25%", "50%", "100%", "150%", "200%", "Fit"});
    m_zoomCombo->setCurrentText("Fit");
    m_zoomCombo->setFixedWidth(70);
    connect(m_zoomCombo, QOverload<int>::of(&QComboBox::activated), this, &CompositionPanel::onZoomChanged);
    
    QLabel* resLabel = new QLabel("Resolution:", this);
    resLabel->setStyleSheet("color: #888888; font-size: 11px;");
    m_resolutionCombo = new QComboBox(this);
    m_resolutionCombo->addItems({"Full", "Half", "Third", "Quarter"});
    m_resolutionCombo->setFixedWidth(70);
    connect(m_resolutionCombo, QOverload<int>::of(&QComboBox::activated), this, &CompositionPanel::onResolutionChanged);
    
    QPushButton* gridBtn = new QPushButton("Grid", this);
    gridBtn->setCheckable(true);
    gridBtn->setStyleSheet(
        "QPushButton { background: transparent; color: #888888; border: none; font-size: 11px; padding: 2px 6px; }"
        "QPushButton:hover { color: #cccccc; }"
        "QPushButton:checked { color: #00d4ff; }"
    );
    
    QPushButton* rulersBtn = new QPushButton("Rulers", this);
    rulersBtn->setCheckable(true);
    rulersBtn->setStyleSheet(gridBtn->styleSheet());
    
    QPushButton* guidesBtn = new QPushButton("Guides", this);
    guidesBtn->setCheckable(true);
    guidesBtn->setStyleSheet(gridBtn->styleSheet());
    
    toolbar->addWidget(zoomLabel);
    toolbar->addWidget(m_zoomCombo);
    toolbar->addSpacing(8);
    toolbar->addWidget(resLabel);
    toolbar->addWidget(m_resolutionCombo);
    toolbar->addSpacing(8);
    toolbar->addWidget(gridBtn);
    toolbar->addWidget(rulersBtn);
    toolbar->addWidget(guidesBtn);
    toolbar->addStretch();
    
    m_mainLayout->addWidget(toolbarWidget);
}

void CompositionPanel::setupBottomBar() {
    QWidget* bottomBar = new QWidget(this);
    bottomBar->setFixedHeight(28);
    bottomBar->setStyleSheet("background-color: #2d2d2d; border-top: 1px solid #1a1a1a;");
    
    QHBoxLayout* bar = new QHBoxLayout(bottomBar);
    bar->setContentsMargins(6, 2, 6, 2);
    bar->setSpacing(8);
    
    m_timeLabel = new QLabel("00:00:00:00", this);
    m_timeLabel->setFont(QFont("Menlo", 10));
    m_timeLabel->setStyleSheet("color: #cccccc; background: transparent;");
    
    m_playButton = new QPushButton(QIcon(":/icons/transport/play.svg"), "", this);
    m_playButton->setFixedSize(24, 22);
    m_playButton->setToolTip("Play/Pause (Space)");
    m_playButton->setStyleSheet(
        "QPushButton { background: transparent; border: none; border-radius: 3px; padding: 2px; }"
        "QPushButton:hover { background-color: #505050; }"
    );
    connect(m_playButton, &QPushButton::clicked, this, &CompositionPanel::onPlayPause);
    
    QPushButton* loopBtn = new QPushButton(QIcon(":/icons/transport/loop.svg"), "", this);
    loopBtn->setFixedSize(22, 22);
    loopBtn->setToolTip("Loop");
    loopBtn->setStyleSheet(m_playButton->styleSheet());
    
    QPushButton* fitBtn = new QPushButton("Fit", this);
    fitBtn->setFixedWidth(40);
    fitBtn->setToolTip("Fit to Window (Ctrl+Shift+/)");
    fitBtn->setStyleSheet(
        "QPushButton { background: transparent; color: #888888; border: none; font-size: 11px; padding: 2px; }"
        "QPushButton:hover { color: #cccccc; }"
    );
    connect(fitBtn, &QPushButton::clicked, this, &CompositionPanel::onFitToWindow);
    
    bar->addWidget(m_timeLabel);
    bar->addStretch();
    bar->addWidget(fitBtn);
    bar->addSpacing(4);
    bar->addWidget(m_playButton);
    bar->addWidget(loopBtn);
    
    m_mainLayout->addWidget(bottomBar);
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
    if (fps <= 0) fps = 30;
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
    double zoomValues[] = {0.25, 0.5, 1.0, 1.5, 2.0};
    if (index >= 0 && index < 5) {
        if (m_canvas) m_canvas->setZoom(zoomValues[index]);
    }
}

void CompositionPanel::onResolutionChanged(int index) {
    Q_UNUSED(index);
}

void CompositionPanel::onPlayPause() {
    m_playing = !m_playing;
    if (m_playing) {
        m_playButton->setIcon(QIcon(":/icons/transport/pause.svg"));
    } else {
        m_playButton->setIcon(QIcon(":/icons/transport/play.svg"));
    }
}

void CompositionPanel::onFitToWindow() {
    if (m_zoomCombo) m_zoomCombo->setCurrentText("Fit");
    if (m_canvas) m_canvas->setZoom(1.0);
}

} // namespace FreeEffect
