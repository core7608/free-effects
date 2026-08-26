#pragma once
#include <QWidget>
#include <QFontComboBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QPushButton>
#include <QColorDialog>
#include <QLabel>
#include <QGridLayout>

namespace FreeEffect {

class CharacterPanel : public QWidget {
    Q_OBJECT
public:
    explicit CharacterPanel(QWidget* parent = nullptr);
    
    void setFont(const QFont& font);
    void setFontSize(double size);
    void setFillColor(const QColor& color);
    void setStrokeColor(const QColor& color);
    void setStrokeWidth(double width);
    void setFauxBold(bool bold);
    void setFauxItalic(bool italic);

signals:
    void fontChanged(const QFont& font);
    void fontSizeChanged(double size);
    void fillColorChanged(const QColor& color);
    void strokeColorChanged(const QColor& color);
    void strokeWidthChanged(double width);
    void fauxBoldToggled(bool bold);
    void fauxItalicToggled(bool italic);
    void trackingChanged(double tracking);
    void leadingChanged(double leading);
    void horizontalScaleChanged(double scale);
    void verticalScaleChanged(double scale);
    void baselineShiftChanged(double shift);

private:
    QFontComboBox* m_fontCombo;
    QComboBox* m_fontStyleCombo;
    QDoubleSpinBox* m_sizeSpinner;
    QDoubleSpinBox* m_leadingSpinner;
    QDoubleSpinBox* m_trackingSpinner;
    QDoubleSpinBox* m_hScaleSpinner;
    QDoubleSpinBox* m_vScaleSpinner;
    QDoubleSpinBox* m_baselineShiftSpinner;
    QPushButton* m_fillColorBtn;
    QPushButton* m_strokeColorBtn;
    QDoubleSpinBox* m_strokeWidthSpinner;
    QCheckBox* m_fauxBoldCheck;
    QCheckBox* m_fauxItalicCheck;
    QPushButton* m_fillNoneBtn;
    QPushButton* m_strokeNoneBtn;
    QPushButton* m_swapFillStrokeBtn;
};

} // namespace FreeEffect
