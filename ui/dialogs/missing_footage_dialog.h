#pragma once

#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "../../core/project/missing_footage_handler.h"

namespace FreeEffect {

class MainWindow;

class MissingFootageDialog : public QDialog {
    Q_OBJECT
public:
    MissingFootageDialog(const std::vector<MissingFootage>& missing, MainWindow* parent);
    
    bool wasRelinked() const { return m_relinked; }

private slots:
    void onRelinkClicked();
    void onRelinkAllClicked();

private:
    void setupUi();
    void refreshList();
    
    MainWindow* m_mainWindow;
    std::vector<MissingFootage> m_missingItems;
    QListWidget* m_listWidget;
    QPushButton* m_relinkBtn;
    QPushButton* m_relinkAllBtn;
    bool m_relinked = false;
};

} // namespace FreeEffect
