#pragma once

#include <QWidget>
#include <QPushButton>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QButtonGroup>

namespace FreeEffect {

class MainWindow;

class AlignPanel : public QWidget {
    Q_OBJECT
public:
    explicit AlignPanel(MainWindow* parent);
    ~AlignPanel() override = default;

signals:
    void alignRequested(int mode);
    void distributeRequested(int mode);
    void distributeSpaceRequested(int mode, double spacing);

private slots:
    void onAlignLeft();
    void onAlignCenterH();
    void onAlignRight();
    void onAlignTop();
    void onAlignMiddle();
    void onAlignBottom();
    void onDistributeH();
    void onDistributeV();
    void onDistributeSpaceH();
    void onDistributeSpaceV();

private:
    void setupUi();

    MainWindow* m_mainWindow;
    QPushButton* m_alignLeftBtn;
    QPushButton* m_alignCenterHBtn;
    QPushButton* m_alignRightBtn;
    QPushButton* m_alignTopBtn;
    QPushButton* m_alignMiddleBtn;
    QPushButton* m_alignBottomBtn;
    QPushButton* m_distributeHBtn;
    QPushButton* m_distributeVBtn;
    QPushButton* m_distributeSpaceHBtn;
    QPushButton* m_distributeSpaceVBtn;
    QDoubleSpinBox* m_spacingSpin;
};

} // namespace FreeEffect
