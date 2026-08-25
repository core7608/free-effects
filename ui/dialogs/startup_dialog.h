#pragma once

#include <QDialog>
#include <QPushButton>
#include <QLabel>
#include <QListWidget>
#include <QVBoxLayout>

namespace FreeEffect {

class StartupDialog : public QDialog {
    Q_OBJECT
public:
    explicit StartupDialog(QWidget* parent = nullptr);
    
    enum Result { NewProject, OpenProject, OpenRecent, Cancel };
    Result getResult() const { return m_result; }
    QString getSelectedRecent() const { return m_selectedRecent; }

private slots:
    void onNewProject();
    void onOpenProject();
    void onRecentDoubleClicked(QListWidgetItem* item);
    void onClose();

private:
    void setupUi();
    void populateRecentProjects();
    
    Result m_result = Cancel;
    QString m_selectedRecent;
    QListWidget* m_recentList;
    QPushButton* m_newBtn;
    QPushButton* m_openBtn;
    QPushButton* m_closeBtn;
};

} // namespace FreeEffect
