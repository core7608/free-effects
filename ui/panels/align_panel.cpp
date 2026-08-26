#include "align_panel.h"
#include "../mainwindow/main_window.h"
#include <QGridLayout>
#include <QFrame>

namespace FreeEffect {

AlignPanel::AlignPanel(MainWindow* parent)
    : QWidget(parent)
    , m_mainWindow(parent) {
    setupUi();
}

void AlignPanel::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(6);

    QGroupBox* alignGroup = new QGroupBox("Align", this);
    QGridLayout* alignLayout = new QGridLayout(alignGroup);
    alignLayout->setSpacing(4);

    m_alignLeftBtn = new QPushButton("Align Left", this);
    m_alignCenterHBtn = new QPushButton("Center H", this);
    m_alignRightBtn = new QPushButton("Align Right", this);
    m_alignTopBtn = new QPushButton("Align Top", this);
    m_alignMiddleBtn = new QPushButton("Center V", this);
    m_alignBottomBtn = new QPushButton("Align Bottom", this);

    alignLayout->addWidget(m_alignLeftBtn, 0, 0);
    alignLayout->addWidget(m_alignCenterHBtn, 0, 1);
    alignLayout->addWidget(m_alignRightBtn, 0, 2);
    alignLayout->addWidget(m_alignTopBtn, 1, 0);
    alignLayout->addWidget(m_alignMiddleBtn, 1, 1);
    alignLayout->addWidget(m_alignBottomBtn, 1, 2);

    connect(m_alignLeftBtn, &QPushButton::clicked, this, &AlignPanel::onAlignLeft);
    connect(m_alignCenterHBtn, &QPushButton::clicked, this, &AlignPanel::onAlignCenterH);
    connect(m_alignRightBtn, &QPushButton::clicked, this, &AlignPanel::onAlignRight);
    connect(m_alignTopBtn, &QPushButton::clicked, this, &AlignPanel::onAlignTop);
    connect(m_alignMiddleBtn, &QPushButton::clicked, this, &AlignPanel::onAlignMiddle);
    connect(m_alignBottomBtn, &QPushButton::clicked, this, &AlignPanel::onAlignBottom);

    mainLayout->addWidget(alignGroup);

    QFrame* sep1 = new QFrame(this);
    sep1->setFrameShape(QFrame::HLine);
    sep1->setFrameShadow(QFrame::Sunken);
    mainLayout->addWidget(sep1);

    QGroupBox* distributeGroup = new QGroupBox("Distribute", this);
    QGridLayout* distLayout = new QGridLayout(distributeGroup);
    distLayout->setSpacing(4);

    m_distributeHBtn = new QPushButton("Distribute H", this);
    m_distributeVBtn = new QPushButton("Distribute V", this);

    distLayout->addWidget(m_distributeHBtn, 0, 0);
    distLayout->addWidget(m_distributeVBtn, 0, 1);

    connect(m_distributeHBtn, &QPushButton::clicked, this, &AlignPanel::onDistributeH);
    connect(m_distributeVBtn, &QPushButton::clicked, this, &AlignPanel::onDistributeV);

    mainLayout->addWidget(distributeGroup);

    QFrame* sep2 = new QFrame(this);
    sep2->setFrameShape(QFrame::HLine);
    sep2->setFrameShadow(QFrame::Sunken);
    mainLayout->addWidget(sep2);

    QGroupBox* spaceGroup = new QGroupBox("Distribute Spacing", this);
    QHBoxLayout* spaceLayout = new QHBoxLayout(spaceGroup);

    m_spacingSpin = new QDoubleSpinBox(this);
    m_spacingSpin->setRange(-10000, 10000);
    m_spacingSpin->setValue(10);
    m_spacingSpin->setSuffix(" px");

    m_distributeSpaceHBtn = new QPushButton("Space H", this);
    m_distributeSpaceVBtn = new QPushButton("Space V", this);

    spaceLayout->addWidget(m_spacingSpin);
    spaceLayout->addWidget(m_distributeSpaceHBtn);
    spaceLayout->addWidget(m_distributeSpaceVBtn);

    connect(m_distributeSpaceHBtn, &QPushButton::clicked, this, &AlignPanel::onDistributeSpaceH);
    connect(m_distributeSpaceVBtn, &QPushButton::clicked, this, &AlignPanel::onDistributeSpaceV);

    mainLayout->addWidget(spaceGroup);
    mainLayout->addStretch();
}

void AlignPanel::onAlignLeft() { emit alignRequested(0); }
void AlignPanel::onAlignCenterH() { emit alignRequested(1); }
void AlignPanel::onAlignRight() { emit alignRequested(2); }
void AlignPanel::onAlignTop() { emit alignRequested(3); }
void AlignPanel::onAlignMiddle() { emit alignRequested(4); }
void AlignPanel::onAlignBottom() { emit alignRequested(5); }
void AlignPanel::onDistributeH() { emit distributeRequested(0); }
void AlignPanel::onDistributeV() { emit distributeRequested(1); }
void AlignPanel::onDistributeSpaceH() { emit distributeSpaceRequested(0, m_spacingSpin->value()); }
void AlignPanel::onDistributeSpaceV() { emit distributeSpaceRequested(1, m_spacingSpin->value()); }

} // namespace FreeEffect
