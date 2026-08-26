#pragma once

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QLineEdit>
#include <QTextEdit>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QTreeWidget>

namespace FreeEffect {

class MainWindow;

class EssentialGraphicsPanel : public QWidget {
    Q_OBJECT
public:
    explicit EssentialGraphicsPanel(MainWindow* parent);
    ~EssentialGraphicsPanel() override = default;

signals:
    void addPropertyRequested();
    void removePropertyRequested();
    void exportMOGRTRequested();
    void previewTemplateRequested();
    void setPropertyRangeRequested(const QString& prop, double min, double max);

private slots:
    void onAddProperty();
    void onRemoveProperty();
    void onExportMOGRT();
    void onPreviewTemplate();
    void onPropertySelected(QTreeWidgetItem* item, int column);

private:
    void setupUi();

    MainWindow* m_mainWindow;
    QTreeWidget* m_propertiesTree;
    QPushButton* m_addPropertyBtn;
    QPushButton* m_removePropertyBtn;
    QPushButton* m_exportMOGRTBtn;
    QPushButton* m_previewBtn;
    QLineEdit* m_nameEdit;
    QTextEdit* m_descriptionEdit;
    QDoubleSpinBox* m_rangeMinSpin;
    QDoubleSpinBox* m_rangeMaxSpin;
    QLabel* m_statusLabel;
};

} // namespace FreeEffect
