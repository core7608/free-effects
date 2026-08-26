#pragma once

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QTreeWidget>
#include <QGroupBox>
#include <QColorDialog>

namespace FreeEffect {

class MainWindow;

class LibrariesPanel : public QWidget {
    Q_OBJECT
public:
    explicit LibrariesPanel(MainWindow* parent);
    ~LibrariesPanel() override = default;

signals:
    void importAssetRequested();
    void deleteAssetRequested();
    void shareAssetRequested();
    void colorSwatchSelected(const QColor& color);
    void graphicStyleApplied(const QString& styleName);

private slots:
    void onImportAsset();
    void onDeleteAsset();
    void onShareAsset();
    void onAddColorSwatch();
    void onAddGraphicStyle();
    void onAssetDoubleClicked(QListWidgetItem* item);

private:
    void setupUi();

    MainWindow* m_mainWindow;
    QComboBox* m_categoryCombo;
    QListWidget* m_assetsList;
    QListWidget* m_swatchesList;
    QListWidget* m_stylesList;
    QPushButton* m_importBtn;
    QPushButton* m_deleteBtn;
    QPushButton* m_shareBtn;
    QPushButton* m_addSwatchBtn;
    QPushButton* m_addStyleBtn;
    QLineEdit* m_searchEdit;
};

} // namespace FreeEffect
