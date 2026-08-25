#pragma once

#include <QDialog>
#include <QTabWidget>
#include <QWidget>
#include <QVBoxLayout>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QSlider>

namespace FreeEffect {

class PreferencesDialog : public QDialog {
    Q_OBJECT
public:
    explicit PreferencesDialog(QWidget* parent = nullptr);

private:
    void setupUi();
    QWidget* createGeneralTab();
    QWidget* createImportTab();
    QWidget* createOutputTab();
    QWidget* createGridsGuidesTab();
    QWidget* createLabelsTab();
    QWidget* createMediaCacheTab();
    QWidget* createMemoryCpuTab();
    QWidget* createAudioHardwareTab();
    
    QTabWidget* m_tabWidget;
};

} // namespace FreeEffect
