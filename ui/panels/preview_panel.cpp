#include "preview_panel.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>

namespace FreeEffect {

static const char* kTransportBtnStyle =
    "QPushButton { background: #3c3c3c; color: #cccccc; border: 1px solid #555555; "
    "border-radius: 3px; font-size: 11px; padding: 4px 8px; }"
    "QPushButton:hover { background: #4a4a4a; border: 1px solid #00d4ff; }"
    "QPushButton:pressed { background: #2a2a2a; }";

static const char* kPlayBtnStyle =
    "QPushButton { background: #00aa44; color: #ffffff; border: 1px solid #00cc55; "
    "border-radius: 3px; font-size: 12px; font-weight: bold; padding: 4px 12px; }"
    "QPushButton:hover { background: #00cc55; }"
    "QPushButton:pressed { background: #008833; }";

static const char* kActiveBtnStyle =
    "QPushButton { background: #00d4ff; color: #000000; border: 1px solid #00d4ff; "
    "border-radius: 3px; font-size: 11px; font-weight: bold; padding: 4px 8px; }"
    "QPushButton:hover { background: #33ddff; }";

PreviewPanel::PreviewPanel(QWidget* parent)
    : QWidget(parent) {
    
    setStyleSheet("background-color: #2a2a2a; color: #cccccc; font-size: 11px;");
    
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(6);
    
    // Preview header
    QLabel* header = new QLabel("Preview");
    header->setStyleSheet("color: #cccccc; font-size: 11px; font-weight: bold;");
    mainLayout->addWidget(header);
    
    // Transport controls
    QWidget* transportRow = new QWidget();
    QHBoxLayout* transportLayout = new QHBoxLayout(transportRow);
    transportLayout->setContentsMargins(0, 0, 0, 0);
    transportLayout->setSpacing(4);
    transportLayout->setAlignment(Qt::AlignCenter);
    
    m_firstFrameBtn = new QPushButton("|<");
    m_firstFrameBtn->setToolTip("First Frame (Home)");
    m_firstFrameBtn->setStyleSheet(kTransportBtnStyle);
    connect(m_firstFrameBtn, &QPushButton::clicked, this, &PreviewPanel::firstFrameClicked);
    transportLayout->addWidget(m_firstFrameBtn);
    
    m_prevFrameBtn = new QPushButton("<");
    m_prevFrameBtn->setToolTip("Previous Frame (Page Up)");
    m_prevFrameBtn->setStyleSheet(kTransportBtnStyle);
    connect(m_prevFrameBtn, &QPushButton::clicked, this, &PreviewPanel::prevFrameClicked);
    transportLayout->addWidget(m_prevFrameBtn);
    
    m_playBtn = new QPushButton("Play");
    m_playBtn->setToolTip("Play/Pause (Space)");
    m_playBtn->setStyleSheet(kPlayBtnStyle);
    connect(m_playBtn, &QPushButton::clicked, this, [this]() {
        if (m_playing) {
            m_playing = false;
            m_playBtn->setText("Play");
            m_playBtn->setStyleSheet(kPlayBtnStyle);
            emit pauseClicked();
        } else {
            m_playing = true;
            m_playBtn->setText("Pause");
            m_playBtn->setStyleSheet(
                "QPushButton { background: #cc8800; color: #ffffff; border: 1px solid #eeaa00; "
                "border-radius: 3px; font-size: 12px; font-weight: bold; padding: 4px 12px; }"
                "QPushButton:hover { background: #eeaa00; }"
                "QPushButton:pressed { background: #aa6600; }");
            emit playClicked();
        }
    });
    transportLayout->addWidget(m_playBtn);
    
    m_nextFrameBtn = new QPushButton(">");
    m_nextFrameBtn->setToolTip("Next Frame (Page Down)");
    m_nextFrameBtn->setStyleSheet(kTransportBtnStyle);
    connect(m_nextFrameBtn, &QPushButton::clicked, this, &PreviewPanel::nextFrameClicked);
    transportLayout->addWidget(m_nextFrameBtn);
    
    m_lastFrameBtn = new QPushButton(">|");
    m_lastFrameBtn->setToolTip("Last Frame (End)");
    m_lastFrameBtn->setStyleSheet(kTransportBtnStyle);
    connect(m_lastFrameBtn, &QPushButton::clicked, this, &PreviewPanel::lastFrameClicked);
    transportLayout->addWidget(m_lastFrameBtn);
    
    m_stopBtn = new QPushButton("Stop");
    m_stopBtn->setToolTip("Stop (Ctrl+.)");
    m_stopBtn->setStyleSheet(kTransportBtnStyle);
    connect(m_stopBtn, &QPushButton::clicked, this, [this]() {
        m_playing = false;
        m_playBtn->setText("Play");
        m_playBtn->setStyleSheet(kPlayBtnStyle);
        emit stopClicked();
    });
    transportLayout->addWidget(m_stopBtn);
    
    m_loopBtn = new QPushButton("Loop");
    m_loopBtn->setToolTip("Toggle Loop");
    m_loopBtn->setCheckable(true);
    m_loopBtn->setChecked(true);
    m_loopBtn->setStyleSheet(kActiveBtnStyle);
    connect(m_loopBtn, &QPushButton::toggled, this, [this](bool checked) {
        m_looping = checked;
        if (checked) {
            m_loopBtn->setStyleSheet(kActiveBtnStyle);
        } else {
            m_loopBtn->setStyleSheet(kTransportBtnStyle);
        }
        emit loopToggled(checked);
    });
    transportLayout->addWidget(m_loopBtn);
    
    mainLayout->addWidget(transportRow);
    
    // Timeline slider and frame label
    QWidget* sliderRow = new QWidget();
    QHBoxLayout* sliderLayout = new QHBoxLayout(sliderRow);
    sliderLayout->setContentsMargins(0, 0, 0, 0);
    sliderLayout->setSpacing(6);
    
    m_frameLabel = new QLabel("0 / 0");
    m_frameLabel->setFixedWidth(80);
    m_frameLabel->setAlignment(Qt::AlignCenter);
    m_frameLabel->setStyleSheet("color: #cccccc; font-size: 11px;");
    sliderLayout->addWidget(m_frameLabel);
    
    m_timelineSlider = new QSlider(Qt::Horizontal);
    m_timelineSlider->setRange(0, 0);
    m_timelineSlider->setValue(0);
    m_timelineSlider->setStyleSheet(
        "QSlider::groove:horizontal { background: #444444; height: 4px; border-radius: 2px; }"
        "QSlider::handle:horizontal { background: #00d4ff; width: 12px; margin: -4px 0; border-radius: 6px; }"
        "QSlider::sub-page:horizontal { background: #00d4ff; border-radius: 2px; }"
    );
    connect(m_timelineSlider, &QSlider::sliderMoved, this, [this](int value) {
        m_frameLabel->setText(QString("%1 / %2").arg(value).arg(m_timelineSlider->maximum()));
        emit frameChanged(value);
    });
    sliderLayout->addWidget(m_timelineSlider, 1);
    
    mainLayout->addWidget(sliderRow);
    
    // Resolution selector
    QWidget* resRow = new QWidget();
    QHBoxLayout* resLayout = new QHBoxLayout(resRow);
    resLayout->setContentsMargins(0, 0, 0, 0);
    resLayout->setSpacing(6);
    
    QLabel* resLabel = new QLabel("Resolution:");
    resLabel->setStyleSheet("color: #888888;");
    resLayout->addWidget(resLabel);
    
    m_resolutionCombo = new QComboBox();
    m_resolutionCombo->addItem("Full", 1.0);
    m_resolutionCombo->addItem("Half", 0.5);
    m_resolutionCombo->addItem("Third", 0.333);
    m_resolutionCombo->addItem("Quarter", 0.25);
    m_resolutionCombo->addItem("Custom", 2.0);
    m_resolutionCombo->setCurrentIndex(0);
    m_resolutionCombo->setStyleSheet(
        "QComboBox { background-color: #2a2a2a; color: #cccccc; border: 1px solid #444444; "
        "border-radius: 2px; padding: 2px 4px; font-size: 11px; }"
        "QComboBox::drop-down { border: none; }"
        "QComboBox QAbstractItemView { background: #2a2a2a; color: #cccccc; selection-background-color: #2a5a7a; }"
    );
    connect(m_resolutionCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        double res = m_resolutionCombo->itemData(index).toDouble();
        emit resolutionChanged(res);
    });
    resLayout->addWidget(m_resolutionCombo);
    resLayout->addStretch();
    
    mainLayout->addWidget(resRow);
}

void PreviewPanel::setPlaying(bool playing) {
    m_playing = playing;
    if (playing) {
        m_playBtn->setText("Pause");
        m_playBtn->setStyleSheet(
            "QPushButton { background: #cc8800; color: #ffffff; border: 1px solid #eeaa00; "
            "border-radius: 3px; font-size: 12px; font-weight: bold; padding: 4px 12px; }"
            "QPushButton:hover { background: #eeaa00; }"
            "QPushButton:pressed { background: #aa6600; }");
    } else {
        m_playBtn->setText("Play");
        m_playBtn->setStyleSheet(kPlayBtnStyle);
    }
}

void PreviewPanel::setCurrentFrame(int frame) {
    m_timelineSlider->blockSignals(true);
    m_timelineSlider->setValue(frame);
    m_timelineSlider->blockSignals(false);
    m_frameLabel->setText(QString("%1 / %2").arg(frame).arg(m_timelineSlider->maximum()));
}

void PreviewPanel::setTotalFrames(int total) {
    m_timelineSlider->setRange(0, total > 0 ? total - 1 : 0);
    m_frameLabel->setText(QString("0 / %1").arg(total > 0 ? total - 1 : 0));
}

void PreviewPanel::setResolution(double resolution) {
    for (int i = 0; i < m_resolutionCombo->count(); ++i) {
        if (qFuzzyCompare(m_resolutionCombo->itemData(i).toDouble(), resolution)) {
            m_resolutionCombo->setCurrentIndex(i);
            break;
        }
    }
}

void PreviewPanel::setFPS(double fps) {
    Q_UNUSED(fps);
}

} // namespace FreeEffect
