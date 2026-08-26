#include "info_panel.h"
#include <QGridLayout>
#include <QVBoxLayout>
#include <QFrame>

namespace FreeEffect {

InfoPanel::InfoPanel(QWidget* parent)
    : QWidget(parent) {
    
    setStyleSheet("background-color: #2a2a2a; color: #cccccc; font-size: 11px;");
    
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(6);
    
    // Composition info section
    QLabel* compHeader = new QLabel("Composition Info");
    compHeader->setStyleSheet("color: #cccccc; font-size: 11px; font-weight: bold;");
    mainLayout->addWidget(compHeader);
    
    QGridLayout* infoGrid = new QGridLayout();
    infoGrid->setSpacing(4);
    
    QLabel* timeNameLabel = new QLabel("Time:");
    timeNameLabel->setStyleSheet("color: #888888;");
    m_timeLabel = new QLabel("0:00:00:00");
    m_timeLabel->setStyleSheet("color: #cccccc;");
    infoGrid->addWidget(timeNameLabel, 0, 0);
    infoGrid->addWidget(m_timeLabel, 0, 1);
    
    QLabel* frameNameLabel = new QLabel("Frame:");
    frameNameLabel->setStyleSheet("color: #888888;");
    m_frameLabel = new QLabel("0");
    m_frameLabel->setStyleSheet("color: #cccccc;");
    infoGrid->addWidget(frameNameLabel, 1, 0);
    infoGrid->addWidget(m_frameLabel, 1, 1);
    
    QLabel* resNameLabel = new QLabel("Resolution:");
    resNameLabel->setStyleSheet("color: #888888;");
    m_resolutionLabel = new QLabel("0 x 0");
    m_resolutionLabel->setStyleSheet("color: #cccccc;");
    infoGrid->addWidget(resNameLabel, 2, 0);
    infoGrid->addWidget(m_resolutionLabel, 2, 1);
    
    QLabel* fpsNameLabel = new QLabel("FPS:");
    fpsNameLabel->setStyleSheet("color: #888888;");
    m_fpsLabel = new QLabel("30.00");
    m_fpsLabel->setStyleSheet("color: #cccccc;");
    infoGrid->addWidget(fpsNameLabel, 3, 0);
    infoGrid->addWidget(m_fpsLabel, 3, 1);
    
    mainLayout->addLayout(infoGrid);
    
    QFrame* sep1 = new QFrame();
    sep1->setFrameShape(QFrame::HLine);
    sep1->setStyleSheet("color: #333333;");
    mainLayout->addWidget(sep1);
    
    // Mouse info section
    QLabel* mouseHeader = new QLabel("Cursor Info");
    mouseHeader->setStyleSheet("color: #cccccc; font-size: 11px; font-weight: bold;");
    mainLayout->addWidget(mouseHeader);
    
    QGridLayout* mouseGrid = new QGridLayout();
    mouseGrid->setSpacing(4);
    
    QLabel* posNameLabel = new QLabel("Position:");
    posNameLabel->setStyleSheet("color: #888888;");
    m_mousePosLabel = new QLabel("0, 0");
    m_mousePosLabel->setStyleSheet("color: #cccccc;");
    mouseGrid->addWidget(posNameLabel, 0, 0);
    mouseGrid->addWidget(m_mousePosLabel, 0, 1);
    
    QLabel* colorNameLabel = new QLabel("Color:");
    colorNameLabel->setStyleSheet("color: #888888;");
    m_colorLabel = new QLabel("--");
    m_colorLabel->setStyleSheet("color: #cccccc;");
    mouseGrid->addWidget(colorNameLabel, 1, 0);
    mouseGrid->addWidget(m_colorLabel, 1, 1);
    
    mainLayout->addLayout(mouseGrid);
    
    QFrame* sep2 = new QFrame();
    sep2->setFrameShape(QFrame::HLine);
    sep2->setStyleSheet("color: #333333;");
    mainLayout->addWidget(sep2);
    
    // Layer info section
    QLabel* layerHeader = new QLabel("Layer Info");
    layerHeader->setStyleSheet("color: #cccccc; font-size: 11px; font-weight: bold;");
    mainLayout->addWidget(layerHeader);
    
    QLabel* layerNameLabel = new QLabel("Layer:");
    layerNameLabel->setStyleSheet("color: #888888;");
    m_layerLabel = new QLabel("None");
    m_layerLabel->setStyleSheet("color: #cccccc;");
    
    QHBoxLayout* layerLayout = new QHBoxLayout();
    layerLayout->addWidget(layerNameLabel);
    layerLayout->addWidget(m_layerLabel);
    layerLayout->addStretch();
    mainLayout->addLayout(layerLayout);
    
    mainLayout->addStretch();
}

void InfoPanel::updateInfo(double time, int frame, int compWidth, int compHeight, double fps) {
    // Format time as HH:MM:SS:FF
    int totalSeconds = static_cast<int>(time);
    int hours = totalSeconds / 3600;
    int minutes = (totalSeconds % 3600) / 60;
    int seconds = totalSeconds % 60;
    int subFrames = static_cast<int>((time - totalSeconds) * fps);
    
    m_timeLabel->setText(QString("%1:%2:%3:%4")
        .arg(hours, 2, 10, QChar('0'))
        .arg(minutes, 2, 10, QChar('0'))
        .arg(seconds, 2, 10, QChar('0'))
        .arg(subFrames, 2, 10, QChar('0')));
    
    m_frameLabel->setText(QString::number(frame));
    m_resolutionLabel->setText(QString("%1 x %2").arg(compWidth).arg(compHeight));
    m_fpsLabel->setText(QString::number(fps, 'f', 2));
}

void InfoPanel::updateMousePosition(int x, int y, const QString& colorInfo) {
    m_mousePosLabel->setText(QString("%1, %2").arg(x).arg(y));
    m_colorLabel->setText(colorInfo.isEmpty() ? "--" : colorInfo);
    emit positionChanged(x, y);
}

void InfoPanel::updateLayerInfo(const QString& layerName, const QString& layerType) {
    if (layerName.isEmpty()) {
        m_layerLabel->setText("None");
    } else {
        m_layerLabel->setText(QString("%1 (%2)").arg(layerName, layerType));
    }
}

} // namespace FreeEffect
