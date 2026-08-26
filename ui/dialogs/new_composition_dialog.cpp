#include "new_composition_dialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QDialogButtonBox>

namespace FreeEffect {

NewCompositionDialog::NewCompositionDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle("Composition Settings");
    setMinimumWidth(450);
    setupUi();
}

void NewCompositionDialog::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    QGridLayout* grid = new QGridLayout();
    
    grid->addWidget(new QLabel("Composition Name:", this), 0, 0);
    m_nameEdit = new QLineEdit("Comp 1", this);
    grid->addWidget(m_nameEdit, 0, 1);
    
    grid->addWidget(new QLabel("Preset:", this), 1, 0);
    m_presetCombo = new QComboBox(this);
    m_presetCombo->addItems({"Custom", "HDTV 1080 29.97", "HDTV 1080 25", "HDTV 1080 30",
                             "HDV/HDTV 720 29.97", "NTSC DV", "PAL D1/DV",
                             "Web 480p", "Web 360p"});
    connect(m_presetCombo, QOverload<int>::of(&QComboBox::activated), this, &NewCompositionDialog::onPresetChanged);
    grid->addWidget(m_presetCombo, 1, 1);
    
    grid->addWidget(new QLabel("Width:", this), 2, 0);
    m_widthSpin = new QSpinBox(this);
    m_widthSpin->setRange(1, 7680);
    m_widthSpin->setValue(1920);
    m_widthSpin->setSuffix(" px");
    grid->addWidget(m_widthSpin, 2, 1);
    
    grid->addWidget(new QLabel("Height:", this), 3, 0);
    m_heightSpin = new QSpinBox(this);
    m_heightSpin->setRange(1, 4320);
    m_heightSpin->setValue(1080);
    m_heightSpin->setSuffix(" px");
    grid->addWidget(m_heightSpin, 3, 1);
    
    m_lockAspectCheck = new QCheckBox("Lock Aspect Ratio to 16:9", this);
    m_lockAspectCheck->setChecked(true);
    grid->addWidget(m_lockAspectCheck, 4, 1);
    
    grid->addWidget(new QLabel("Pixel Aspect Ratio:", this), 5, 0);
    m_pixelAspectCombo = new QComboBox(this);
    m_pixelAspectCombo->addItems({"Square Pixels (1.0)", "D1/DV NTSC (0.91)", "D1/DV PAL (1.09)"});
    grid->addWidget(m_pixelAspectCombo, 5, 1);
    
    grid->addWidget(new QLabel("Frame Rate:", this), 6, 0);
    m_frameRateSpin = new QDoubleSpinBox(this);
    m_frameRateSpin->setRange(1.0, 240.0);
    m_frameRateSpin->setValue(30.0);
    m_frameRateSpin->setSuffix(" fps");
    grid->addWidget(m_frameRateSpin, 6, 1);
    
    grid->addWidget(new QLabel("Resolution:", this), 7, 0);
    m_resolutionCombo = new QComboBox(this);
    m_resolutionCombo->addItems({"Full", "Half", "Third", "Quarter"});
    grid->addWidget(m_resolutionCombo, 7, 1);
    
    grid->addWidget(new QLabel("Start Timecode:", this), 8, 0);
    m_startTimeSpin = new QDoubleSpinBox(this);
    m_startTimeSpin->setRange(0.0, 86400.0);
    m_startTimeSpin->setDecimals(2);
    m_startTimeSpin->setSuffix(" sec");
    grid->addWidget(m_startTimeSpin, 8, 1);
    
    grid->addWidget(new QLabel("Duration:", this), 9, 0);
    m_durationSpin = new QDoubleSpinBox(this);
    m_durationSpin->setRange(0.1, 86400.0);
    m_durationSpin->setValue(10.0);
    m_durationSpin->setDecimals(2);
    m_durationSpin->setSuffix(" sec");
    grid->addWidget(m_durationSpin, 9, 1);
    
    grid->addWidget(new QLabel("Background Color:", this), 10, 0);
    m_bgColorBtn = new QPushButton("  ", this);
    m_bgColorBtn->setStyleSheet("background-color: black;");
    grid->addWidget(m_bgColorBtn, 10, 1, Qt::AlignLeft);
    
    mainLayout->addLayout(grid);
    
    QDialogButtonBox* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    m_okBtn = buttons->button(QDialogButtonBox::Ok);
    m_cancelBtn = buttons->button(QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this, &NewCompositionDialog::onAccepted);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttons);
}

void NewCompositionDialog::onPresetChanged(int index) {
    switch (index) {
        case 1: m_widthSpin->setValue(1920); m_heightSpin->setValue(1080); m_frameRateSpin->setValue(29.97); break;
        case 2: m_widthSpin->setValue(1920); m_heightSpin->setValue(1080); m_frameRateSpin->setValue(25.0); break;
        case 3: m_widthSpin->setValue(1920); m_heightSpin->setValue(1080); m_frameRateSpin->setValue(30.0); break;
        case 4: m_widthSpin->setValue(1280); m_heightSpin->setValue(720); m_frameRateSpin->setValue(29.97); break;
        case 5: m_widthSpin->setValue(720); m_heightSpin->setValue(480); m_frameRateSpin->setValue(29.97); break;
        case 6: m_widthSpin->setValue(720); m_heightSpin->setValue(576); m_frameRateSpin->setValue(25.0); break;
        case 7: m_widthSpin->setValue(854); m_heightSpin->setValue(480); m_frameRateSpin->setValue(30.0); break;
        case 8: m_widthSpin->setValue(640); m_heightSpin->setValue(360); m_frameRateSpin->setValue(30.0); break;
    }
}

void NewCompositionDialog::onAccepted() {
    accept();
}

QString NewCompositionDialog::getCompositionName() const { return m_nameEdit->text(); }
int NewCompositionDialog::getWidth() const { return m_widthSpin->value(); }
int NewCompositionDialog::getHeight() const { return m_heightSpin->value(); }
double NewCompositionDialog::getFrameRate() const { return m_frameRateSpin->value(); }
double NewCompositionDialog::getDuration() const { return m_durationSpin->value(); }

void NewCompositionDialog::setCompositionName(const QString& name) { m_nameEdit->setText(name); }
void NewCompositionDialog::setWidth(int w) { m_widthSpin->setValue(w); }
void NewCompositionDialog::setHeight(int h) { m_heightSpin->setValue(h); }
void NewCompositionDialog::setFrameRate(double fps) { m_frameRateSpin->setValue(fps); }
void NewCompositionDialog::setDuration(double dur) { m_durationSpin->setValue(dur); }

} // namespace FreeEffect
