#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>

namespace FreeEffect {

class NewCompositionDialog : public QDialog {
    Q_OBJECT
public:
    explicit NewCompositionDialog(QWidget* parent = nullptr);
    
    QString getCompositionName() const;
    int getWidth() const;
    int getHeight() const;
    double getFrameRate() const;
    double getDuration() const;

private slots:
    void onPresetChanged(int index);
    void onAccepted();

private:
    void setupUi();
    
    QLineEdit* m_nameEdit;
    QComboBox* m_presetCombo;
    QSpinBox* m_widthSpin;
    QSpinBox* m_heightSpin;
    QCheckBox* m_lockAspectCheck;
    QComboBox* m_pixelAspectCombo;
    QDoubleSpinBox* m_frameRateSpin;
    QComboBox* m_resolutionCombo;
    QDoubleSpinBox* m_startTimeSpin;
    QDoubleSpinBox* m_durationSpin;
    QPushButton* m_bgColorBtn;
    QPushButton* m_okBtn;
    QPushButton* m_cancelBtn;
};

} // namespace FreeEffect
