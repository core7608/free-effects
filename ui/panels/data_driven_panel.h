#pragma once

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QTreeWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QGroupBox>

namespace FreeEffect {

class MainWindow;

class DataDrivenPanel : public QWidget {
    Q_OBJECT
public:
    explicit DataDrivenPanel(MainWindow* parent);
    ~DataDrivenPanel() override = default;

signals:
    void importJSONRequested();
    void bindPropertyRequested(const QString& dataPath, const QString& propertyPath);
    void refreshDataRequested();

private slots:
    void onImportJSON();
    void onBindProperty();
    void onRefreshData();
    void onTreeItemClicked(QTreeWidgetItem* item, int column);

private:
    void setupUi();
    void populateTree(const QString& jsonStr);

    MainWindow* m_mainWindow;
    QTreeWidget* m_dataTree;
    QLineEdit* m_dataPathEdit;
    QLineEdit* m_propertyPathEdit;
    QPushButton* m_importBtn;
    QPushButton* m_bindBtn;
    QPushButton* m_refreshBtn;
    QLabel* m_previewLabel;
};

} // namespace FreeEffect
