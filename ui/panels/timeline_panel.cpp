#include "timeline_panel.h"
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
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    QSplitter* splitter = new QSplitter(Qt::Horizontal, this);
    
    // Layer list (left side)
    QWidget* layerPanel = new QWidget();
    QVBoxLayout* layerLayout = new QVBoxLayout(layerPanel);
    layerLayout->setContentsMargins(0, 0, 0, 0);
    
    setupLayerList();
    layerLayout->addWidget(m_layerTree);
    splitter->addWidget(layerPanel);
    
    // Timeline view (right side)
    QWidget* timePanel = new QWidget();
    QVBoxLayout* timeLayout = new QVBoxLayout(timePanel);
    timeLayout->setContentsMargins(0, 0, 0, 0);
    
    setupTimelineView();
    timeLayout->addWidget(m_timelineView);
    splitter->addWidget(timePanel);
    
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 3);
    
    mainLayout->addWidget(splitter, 1);
    
    // Transport controls at bottom
    setupTransportControls();
}

void TimelinePanel::setupLayerList() {
    m_layerTree = new QTreeWidget(this);
    QStringList headers;
    headers << "" << "" << "" << "Layer Name" << "Parent";
    m_layerTree->setHeaderLabels(headers);
    m_layerTree->setColumnWidth(0, 24);
    m_layerTree->setColumnWidth(1, 24);
    m_layerTree->setColumnWidth(2, 24);
    m_layerTree->setColumnWidth(3, 150);
    m_layerTree->setColumnWidth(4, 80);
    m_layerTree->setRootIsDecorated(false);
    m_layerTree->setAlternatingRowColors(true);
    m_layerTree->setSelectionMode(QAbstractItemView::SingleSelection);
    m_layerTree->setContextMenuPolicy(Qt::CustomContextMenu);
}

void TimelinePanel::setupTimelineView() {
    m_timelineView = new QWidget(this);
    m_timelineView->setMinimumWidth(400);
    m_timelineView->setAutoFillBackground(true);
    
    QPalette pal = m_timelineView->palette();
    pal.setColor(QPalette::Window, QColor(45, 45, 45));
    m_timelineView->setPalette(pal);
}

void TimelinePanel::setupTransportControls() {
    QHBoxLayout* transport = new QHBoxLayout();
    transport->setContentsMargins(4, 2, 4, 2);
    
    QPushButton* goStartBtn = new QPushButton("<<", this);
    QPushButton* prevFrameBtn = new QPushButton("<", this);
    m_playButton = new QPushButton("Play", this);
    QPushButton* nextFrameBtn = new QPushButton(">", this);
    QPushButton* goEndBtn = new QPushButton(">>", this);
    
    m_timeLabel = new QLabel("00:00:00:00", this);
    m_timeLabel->setFont(QFont("Monospace", 10));
    
    m_timeSlider = new QSlider(Qt::Horizontal, this);
    m_timeSlider->setMinimum(0);
    m_timeSlider->setMaximum(1000);
    
    connect(m_playButton, &QPushButton::clicked, this, &TimelinePanel::onPlayPause);
    connect(goStartBtn, &QPushButton::clicked, this, &TimelinePanel::onGoToStart);
    connect(goEndBtn, &QPushButton::clicked, this, &TimelinePanel::onGoToEnd);
    
    transport->addWidget(goStartBtn);
    transport->addWidget(prevFrameBtn);
    transport->addWidget(m_playButton);
    transport->addWidget(nextFrameBtn);
    transport->addWidget(goEndBtn);
    transport->addSpacing(10);
    transport->addWidget(m_timeLabel);
    transport->addSpacing(10);
    transport->addWidget(m_timeSlider);
    
    layout()->addItem(transport);
}

void TimelinePanel::setComposition(std::shared_ptr<Composition> comp, CommandStack* stack) {
    m_composition = comp;
    m_commandStack = stack;
    m_currentTime = 0.0;
    m_selectedLayerIndex = -1;
    refreshLayerList();
    updateTimeDisplay();
}

void TimelinePanel::refreshTimeline() {
    refreshLayerList();
}

void TimelinePanel::refreshLayerList() {
    m_layerTree->clear();
    if (!m_composition) return;
    
    const auto& layers = m_composition->getLayers();
    for (int i = 0; i < static_cast<int>(layers.size()); ++i) {
        const auto& layer = layers[i];
        QTreeWidgetItem* item = new QTreeWidgetItem(m_layerTree);
        
        // Eye icon (video visibility)
        QTreeWidgetItem* eyeItem = new QTreeWidgetItem(item);
        eyeItem->setText(0, layer->isVisible() ? "V" : "");
        
        // Audio icon
        QTreeWidgetItem* audioItem = new QTreeWidgetItem(item);
        audioItem->setText(0, layer->isAudioEnabled() ? "A" : "");
        
        // Solo
        QTreeWidgetItem* soloItem = new QTreeWidgetItem(item);
        soloItem->setText(0, layer->isSolo() ? "S" : "");
        
        item->setText(3, QString::fromStdString(layer->getName()));
        item->setText(4, QString::fromStdString(layer->getParentLayerId()));
        item->setData(3, Qt::UserRole, i);
    }
}

void TimelinePanel::setCurrentTime(double timeInSeconds) {
    m_currentTime = timeInSeconds;
    updateTimeDisplay();
    emit currentTimeChanged(timeInSeconds);
}

void TimelinePanel::updateTimeDisplay() {
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

void TimelinePanel::onPlayPause() {
    m_playing = !m_playing;
    m_playButton->setText(m_playing ? "Pause" : "Play");
}

void TimelinePanel::onGoToStart() {
    setCurrentTime(0.0);
}

void TimelinePanel::onGoToEnd() {
    if (m_composition) setCurrentTime(m_composition->getDuration());
}

void TimelinePanel::onZoomIn() {}
void TimelinePanel::onZoomOut() {}
void TimelinePanel::onLayerClicked(int row, int column) {
    Q_UNUSED(column);
    m_selectedLayerIndex = row;
    emit layerSelected(row);
}

} // namespace FreeEffect
