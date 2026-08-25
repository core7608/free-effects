#include "timeline_panel.h"
#include "timeline_canvas.h"
#include "../mainwindow/main_window.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QSplitter>

namespace FreeEffect {

TimelinePanel::TimelinePanel(MainWindow* parent)
    : QWidget(parent)
    , m_mainWindow(parent) {
    setupUi();
}

void TimelinePanel::setupUi() {
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    
    QSplitter* splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setStyleSheet("QSplitter::handle { background: #1a1a1a; }");
    
    // Layer list (left side)
    setupLayerList();
    splitter->addWidget(m_layerTree);
    
    // Timeline view (right side)
    setupTimelineView();
    splitter->addWidget(m_timelineView);
    
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 3);
    
    m_mainLayout->addWidget(splitter, 1);
    
    // Transport controls at bottom
    setupTransportControls();
}

void TimelinePanel::setupLayerList() {
    m_layerTree = new QTreeWidget(this);
    m_layerTree->setObjectName("TimelineLayerTree");
    m_layerTree->setHeaderLabels(QStringList() << "" << "" << "" << "Layer Name" << "Source Name" << "Parent");
    m_layerTree->setColumnWidth(0, 24);  // Eye
    m_layerTree->setColumnWidth(1, 24);  // Audio
    m_layerTree->setColumnWidth(2, 24);  // Solo/Lock
    m_layerTree->setColumnWidth(3, 150); // Layer Name
    m_layerTree->setColumnWidth(4, 100); // Source Name
    m_layerTree->setColumnWidth(5, 60);  // Parent
    m_layerTree->setRootIsDecorated(false);
    m_layerTree->setAlternatingRowColors(false);
    m_layerTree->setSelectionMode(QAbstractItemView::SingleSelection);
    m_layerTree->setContextMenuPolicy(Qt::CustomContextMenu);
    m_layerTree->setIndentation(0);
    m_layerTree->setStyleSheet(
        "QTreeWidget { background-color: #2a2a2a; color: #cccccc; border: none; "
        "font-size: 11px; }"
        "QTreeWidget::item { padding: 1px; border: none; height: 22px; }"
        "QTreeWidget::item:selected { background-color: #2680eb; color: white; }"
        "QTreeWidget::item:hover { background-color: #353535; }"
        "QHeaderView::section { background-color: #2d2d2d; color: #888888; border: none; "
        "border-right: 1px solid #1a1a1a; padding: 3px 4px; font-size: 10px; }"
    );
    
    connect(m_layerTree, &QTreeWidget::itemClicked, this, &TimelinePanel::onLayerClicked);
}

void TimelinePanel::setupTimelineView() {
    m_timelineView = new QWidget(this);
    m_timelineView->setMinimumWidth(400);
    
    QVBoxLayout* layout = new QVBoxLayout(m_timelineView);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    
    m_timelineCanvas = new TimelineCanvas(m_timelineView);
    layout->addWidget(m_timelineCanvas);
    
    connect(m_timelineCanvas, &TimelineCanvas::timeClicked, this, &TimelinePanel::onCanvasTimeClicked);
    connect(m_timelineCanvas, &TimelineCanvas::layerClicked, this, &TimelinePanel::onCanvasLayerClicked);
    connect(m_timelineCanvas, &TimelineCanvas::zoomChanged, this, [this](double pps) {
        Q_UNUSED(pps);
        // Could update zoom indicator
    });
}

void TimelinePanel::setupTransportControls() {
    QWidget* transportBar = new QWidget(this);
    transportBar->setFixedHeight(32);
    transportBar->setStyleSheet("background-color: #333333; border-top: 1px solid #1a1a1a;");
    
    QHBoxLayout* transport = new QHBoxLayout(transportBar);
    transport->setContentsMargins(6, 2, 6, 2);
    transport->setSpacing(2);
    
    auto createTransportBtn = [this](const QString& icon, const QString& tooltip) -> QPushButton* {
        QPushButton* btn = new QPushButton(QIcon(icon), "", this);
        btn->setToolTip(tooltip);
        btn->setFixedSize(26, 24);
        btn->setStyleSheet(
            "QPushButton { background: transparent; border: none; border-radius: 3px; padding: 2px; }"
            "QPushButton:hover { background-color: #505050; }"
            "QPushButton:pressed { background-color: #383838; }"
        );
        return btn;
    };
    
    QPushButton* goStartBtn = createTransportBtn(":/icons/transport/go_start.svg", "Go to Start (Home)");
    QPushButton* prevFrameBtn = createTransportBtn(":/icons/transport/prev_frame.svg", "Previous Frame (,)");
    
    m_playButton = createTransportBtn(":/icons/transport/play.svg", "Play/Pause (Space)");
    m_playButton->setFixedSize(30, 24);
    connect(m_playButton, &QPushButton::clicked, this, &TimelinePanel::onPlayPause);
    
    QPushButton* nextFrameBtn = createTransportBtn(":/icons/transport/next_frame.svg", "Next Frame (.)");
    QPushButton* goEndBtn = createTransportBtn(":/icons/transport/go_end.svg", "Go to End (End)");
    QPushButton* loopBtn = createTransportBtn(":/icons/transport/loop.svg", "Loop");
    
    m_timeLabel = new QLabel("00:00:00:00", this);
    m_timeLabel->setFixedWidth(85);
    m_timeLabel->setFont(QFont("Menlo", 10));
    m_timeLabel->setAlignment(Qt::AlignCenter);
    m_timeLabel->setStyleSheet("color: #cccccc; background-color: #2a2a2a; border: 1px solid #444444; border-radius: 2px; padding: 2px 4px;");
    
    m_timeSlider = new QSlider(Qt::Horizontal, this);
    m_timeSlider->setMinimum(0);
    m_timeSlider->setMaximum(1000);
    m_timeSlider->setStyleSheet(
        "QSlider::groove:horizontal { height: 4px; background: #444444; border-radius: 2px; }"
        "QSlider::handle:horizontal { width: 10px; height: 10px; margin: -3px 0; background: #cccccc; border-radius: 5px; }"
        "QSlider::sub-page:horizontal { background: #2680eb; border-radius: 2px; }"
    );
    
    connect(goStartBtn, &QPushButton::clicked, this, &TimelinePanel::onGoToStart);
    connect(goEndBtn, &QPushButton::clicked, this, &TimelinePanel::onGoToEnd);
    connect(prevFrameBtn, &QPushButton::clicked, this, &TimelinePanel::onStepBackward);
    connect(nextFrameBtn, &QPushButton::clicked, this, &TimelinePanel::onStepForward);
    
    transport->addWidget(goStartBtn);
    transport->addWidget(prevFrameBtn);
    transport->addWidget(m_playButton);
    transport->addWidget(nextFrameBtn);
    transport->addWidget(goEndBtn);
    transport->addWidget(loopBtn);
    transport->addSpacing(8);
    transport->addWidget(m_timeLabel);
    transport->addSpacing(8);
    transport->addWidget(m_timeSlider, 1);
    
    m_mainLayout->addWidget(transportBar);
    
    // Play timer
    m_playTimer = new QTimer(this);
    connect(m_playTimer, &QTimer::timeout, this, &TimelinePanel::advanceFrame);
}

void TimelinePanel::setComposition(std::shared_ptr<Composition> comp, CommandStack* stack) {
    m_composition = comp;
    m_commandStack = stack;
    m_currentTime = 0.0;
    m_selectedLayerIndex = -1;
    refreshLayerList();
    updateTimeDisplay();
    
    if (m_timeSlider && comp) {
        m_timeSlider->setMaximum(static_cast<int>(comp->getDuration() * comp->getFrameRate().fps));
    }
    
    if (m_timelineCanvas) {
        m_timelineCanvas->setComposition(comp);
        m_timelineCanvas->setCurrentTime(0.0);
    }
}

void TimelinePanel::refreshTimeline() {
    refreshLayerList();
    updateTimeDisplay();
    if (m_timelineCanvas) m_timelineCanvas->update();
}

void TimelinePanel::refreshLayerList() {
    m_layerTree->clear();
    if (!m_composition) return;
    
    const auto& layers = m_composition->getLayers();
    for (int i = static_cast<int>(layers.size()) - 1; i >= 0; --i) {
        const auto& layer = layers[i];
        QTreeWidgetItem* item = new QTreeWidgetItem(m_layerTree);
        
        // Column 0: Eye (video visibility)
        item->setText(0, layer->isVisible() ? "👁" : "");
        item->setTextAlignment(0, Qt::AlignCenter);
        
        // Column 1: Audio
        item->setText(1, layer->isAudioEnabled() ? "🔊" : "");
        item->setTextAlignment(1, Qt::AlignCenter);
        
        // Column 2: Solo/Lock
        item->setText(2, layer->isSolo() ? "S" : "");
        item->setTextAlignment(2, Qt::AlignCenter);
        
        // Column 3: Layer Name
        item->setText(3, QString::fromStdString(layer->getName()));
        
        // Column 4: Source Name (same as name for now)
        item->setText(4, QString::fromStdString(layer->getName()));
        
        // Column 5: Parent
        item->setText(5, QString::fromStdString(layer->getParentLayerId()));
        
        item->setData(3, Qt::UserRole, i);
        
        // Set color based on layer type
        QColor layerColor(200, 200, 200);
        item->setForeground(3, layerColor);
        item->setForeground(4, layerColor);
    }
    
    if (m_timelineCanvas) m_timelineCanvas->update();
}

void TimelinePanel::setCurrentTime(double timeInSeconds) {
    m_currentTime = timeInSeconds;
    updateTimeDisplay();
    if (m_timeSlider && m_composition) {
        int frame = static_cast<int>(m_currentTime * m_composition->getFrameRate().fps);
        m_timeSlider->blockSignals(true);
        m_timeSlider->setValue(frame);
        m_timeSlider->blockSignals(false);
    }
    if (m_timelineCanvas) m_timelineCanvas->setCurrentTime(timeInSeconds);
    emit currentTimeChanged(timeInSeconds);
}

void TimelinePanel::updateTimeDisplay() {
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

void TimelinePanel::advanceFrame() {
    if (!m_composition || !m_playing) return;
    
    double frameDuration = 1.0 / m_composition->getFrameRate().fps;
    m_currentTime += frameDuration * m_frameDelta;
    
    double duration = m_composition->getDuration();
    if (m_currentTime >= duration) {
        m_currentTime = 0.0;
    } else if (m_currentTime < 0.0) {
        m_currentTime = duration;
    }
    
    updateTimeDisplay();
    emit currentTimeChanged(m_currentTime);
}

void TimelinePanel::onPlayPause() {
    m_playing = !m_playing;
    
    if (m_playing) {
        m_playButton->setIcon(QIcon(":/icons/transport/pause.svg"));
        m_playButton->setToolTip("Pause (Space)");
        m_frameDelta = 1;
        if (m_composition) {
            double frameDuration = 1000.0 / m_composition->getFrameRate().fps;
            m_playTimer->start(static_cast<int>(frameDuration));
        }
    } else {
        m_playButton->setIcon(QIcon(":/icons/transport/play.svg"));
        m_playButton->setToolTip("Play (Space)");
        m_playTimer->stop();
    }
}

void TimelinePanel::onGoToStart() {
    setCurrentTime(0.0);
}

void TimelinePanel::onGoToEnd() {
    if (m_composition) setCurrentTime(m_composition->getDuration());
}

void TimelinePanel::onStepForward() {
    if (!m_composition) return;
    double frameDuration = 1.0 / m_composition->getFrameRate().fps;
    setCurrentTime(std::min(m_currentTime + frameDuration, m_composition->getDuration()));
}

void TimelinePanel::onStepBackward() {
    if (!m_composition) return;
    double frameDuration = 1.0 / m_composition->getFrameRate().fps;
    setCurrentTime(std::max(m_currentTime - frameDuration, 0.0));
}

void TimelinePanel::onZoomIn() {}
void TimelinePanel::onZoomOut() {}

void TimelinePanel::onLayerClicked(QTreeWidgetItem* item, int column) {
    Q_UNUSED(column);
    if (!item) return;
    
    QVariant data = item->data(3, Qt::UserRole);
    if (data.isValid()) {
        m_selectedLayerIndex = data.toInt();
        if (m_timelineCanvas) m_timelineCanvas->setSelectedLayer(m_selectedLayerIndex);
        emit layerSelected(m_selectedLayerIndex);
    }
}

void TimelinePanel::onCanvasTimeClicked(double time) {
    setCurrentTime(time);
}

void TimelinePanel::onCanvasLayerClicked(int index) {
    m_selectedLayerIndex = index;
    if (m_timelineCanvas) m_timelineCanvas->setSelectedLayer(index);
    
    // Sync with tree widget (select the corresponding item)
    if (m_composition) {
        int treeRow = m_composition->getLayerCount() - 1 - index;
        if (treeRow >= 0 && treeRow < m_layerTree->topLevelItemCount()) {
            m_layerTree->setCurrentItem(m_layerTree->topLevelItem(treeRow));
        }
    }
    
    emit layerSelected(index);
}

} // namespace FreeEffect
