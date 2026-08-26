#pragma once

#include <QWidget>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QRadioButton>
#include <QGroupBox>
#include <QCheckBox>

namespace FreeEffect {

class MainWindow;

class ContentAwareFillPanel : public QWidget {
    Q_OBJECT
public:
    explicit ContentAwareFillPanel(MainWindow* parent);
    ~ContentAwareFillPanel() override = default;

signals:
    void createReferenceFrameRequested();
    void generateFillLayerRequested();
    void cancelRequested();

private slots:
    void onCreateReferenceFrame();
    void onGenerateFill();
    void onCancel();
    void onMethodChanged(int index);

private:
    void setupUi();

    MainWindow* m_mainWindow;
    QComboBox* m_methodCombo;
    QComboBox* m_rangeCombo;
    QComboBox* m_fillTargetCombo;
    QPushButton* m_createRefFrameBtn;
    QPushButton* m_generateFillBtn;
    QPushButton* m_cancelBtn;
    QProgressBar* m_progressBar;
    QLabel* m_statusLabel;
    QCheckBox* m_fillEmptyCheck;
};

} // namespace FreeEffect
