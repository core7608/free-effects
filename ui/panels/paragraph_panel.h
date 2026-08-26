#pragma once
#include <QWidget>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QHBoxLayout>

namespace FreeEffect {

class ParagraphPanel : public QWidget {
    Q_OBJECT
public:
    explicit ParagraphPanel(QWidget* parent = nullptr);
    
    void setJustification(int mode);
    void setIndent(double indent);
    void setLeftMargin(double margin);
    void setRightMargin(double margin);
    void setSpaceBefore(double space);
    void setSpaceAfter(double space);

signals:
    void justificationChanged(int mode);
    void indentChanged(double indent);
    void leftMarginChanged(double margin);
    void rightMarginChanged(double margin);
    void spaceBeforeChanged(double space);
    void spaceAfterChanged(double space);

private:
    QPushButton* m_leftAlignBtn;
    QPushButton* m_centerAlignBtn;
    QPushButton* m_rightAlignBtn;
    QPushButton* m_lastLeftBtn;
    QPushButton* m_lastCenterBtn;
    QPushButton* m_lastRightBtn;
    QDoubleSpinBox* m_indentSpinner;
    QDoubleSpinBox* m_leftMarginSpinner;
    QDoubleSpinBox* m_rightMarginSpinner;
    QDoubleSpinBox* m_spaceBeforeSpinner;
    QDoubleSpinBox* m_spaceAfterSpinner;
};

} // namespace FreeEffect
