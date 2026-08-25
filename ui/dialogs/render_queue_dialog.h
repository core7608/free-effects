#pragma once

#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QProgressBar>

namespace FreeEffect {

struct RenderQueueItem {
    QString compositionName;
    QString outputPath;
    QString status;
    double progress = 0.0;
};

class RenderQueueDialog : public QDialog {
    Q_OBJECT
public:
    explicit RenderQueueDialog(QWidget* parent = nullptr);
    
    void addItem(const QString& compositionName, const QString& outputPath);
    void updateItemStatus(int index, const QString& status, double progress = 0.0);

private slots:
    void onRenderClicked();
    void onRemoveClicked();

private:
    void setupUi();
    
    QListWidget* m_queueList;
    QPushButton* m_renderBtn;
    QPushButton* m_removeBtn;
    QProgressBar* m_progressBar;
    QLabel* m_statusLabel;
    QList<RenderQueueItem> m_items;
};

} // namespace FreeEffect
