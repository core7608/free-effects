#include "planar_editor_panel.h"
#include "../mainwindow/main_window.h"
#include <QGridLayout>
#include <QFrame>
#include <QGroupBox>

namespace FreeEffect {

PlanarEditorPanel::PlanarEditorPanel(MainWindow* parent)
    : QWidget(parent)
    , m_mainWindow(parent) {
    setupUi();
}

void PlanarEditorPanel::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(6);

    QGroupBox* surfaceGroup = new QGroupBox("Surface", this);
    QVBoxLayout* sLayout = new QVBoxLayout(surfaceGroup);

    m_surfaceModeCombo = new QComboBox(this);
    m_surfaceModeCombo->addItem("X-Sheet Style");
    m_surfaceModeCombo->addItem("Power Search");
    sLayout->addWidget(m_surfaceModeCombo);

    m_trackForCombo = new QComboBox(this);
    m_trackForCombo->addItem("Translation");
    m_trackForCombo->addItem("Translation + Scale");
    m_trackForCombo->addItem("Translation + Scale + Rotation");
    m_trackForCombo->addItem("Perspective");
    sLayout->addWidget(m_trackForCombo);

    connect(m_surfaceModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &PlanarEditorPanel::onSurfaceModeChanged);

    mainLayout->addWidget(surfaceGroup);

    QGroupBox* pointsGroup = new QGroupBox("Tracking Points", this);
    QVBoxLayout* pLayout = new QVBoxLayout(pointsGroup);

    m_pointList = new QListWidget(this);
    m_pointList->setMaximumHeight(100);
    for (int i = 1; i <= 4; ++i) {
        m_pointList->addItem(QString("Point %1: (0, 0)").arg(i));
    }
    pLayout->addWidget(m_pointList);

    m_adjustPointsBtn = new QPushButton("Adjust Tracking Points", this);
    m_resetSurfaceBtn = new QPushButton("Reset Surface", this);
    pLayout->addWidget(m_adjustPointsBtn);
    pLayout->addWidget(m_resetSurfaceBtn);

    connect(m_adjustPointsBtn, &QPushButton::clicked, this, &PlanarEditorPanel::onAdjustPoints);
    connect(m_resetSurfaceBtn, &QPushButton::clicked, this, &PlanarEditorPanel::onResetSurface);

    mainLayout->addWidget(pointsGroup);

    QFrame* sep = new QFrame(this);
    sep->setFrameShape(QFrame::HLine);
    sep->setFrameShadow(QFrame::Sunken);
    mainLayout->addWidget(sep);

    QGroupBox* trackGroup = new QGroupBox("Tracking", this);
    QVBoxLayout* tLayout = new QVBoxLayout(trackGroup);

    m_trackForwardBtn = new QPushButton("Track Forward", this);
    m_trackBackwardBtn = new QPushButton("Track Backward", this);
    m_exportToAEBtn = new QPushButton("Export Shape to AE", this);

    tLayout->addWidget(m_trackForwardBtn);
    tLayout->addWidget(m_trackBackwardBtn);
    tLayout->addWidget(m_exportToAEBtn);

    connect(m_trackForwardBtn, &QPushButton::clicked, this, &PlanarEditorPanel::onTrackForward);
    connect(m_trackBackwardBtn, &QPushButton::clicked, this, &PlanarEditorPanel::onTrackBackward);
    connect(m_exportToAEBtn, &QPushButton::clicked, this, &PlanarEditorPanel::onExportToAE);

    mainLayout->addWidget(trackGroup);

    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setVisible(false);
    mainLayout->addWidget(m_progressBar);

    mainLayout->addStretch();
}

void PlanarEditorPanel::onTrackForward() {
    m_progressBar->setVisible(true);
    emit trackForwardRequested();
}
void PlanarEditorPanel::onTrackBackward() {
    m_progressBar->setVisible(true);
    emit trackBackwardRequested();
}
void PlanarEditorPanel::onExportToAE() { emit exportToAERequested(); }
void PlanarEditorPanel::onAdjustPoints() { emit adjustPointsRequested(); }
void PlanarEditorPanel::onResetSurface() { emit resetSurfaceRequested(); }
void PlanarEditorPanel::onSurfaceModeChanged(int index) {}

} // namespace FreeEffect
