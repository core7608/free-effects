#pragma once
#include <QWidget>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>

namespace FreeEffect {

class PosterizeTimePanel : public QWidget {
    Q_OBJECT
public:
    explicit PosterizeTimePanel(QWidget* parent = nullptr);

signals:
    void applyPosterizeTime(double frameRate);

private:
    QDoubleSpinBox* m_frameRateSpinner;
    QPushButton* m_applyBtn;
};

} // namespace FreeEffect
