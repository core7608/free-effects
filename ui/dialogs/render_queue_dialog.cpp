#include "render_queue_dialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>

namespace FreeEffect {

RenderQueueDialog::RenderQueueDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle("Render Queue");
    setMinimumSize(500, 400);
    setupUi();
}

void RenderQueueDialog::setupUi() {
    QVBoxLayout* layout = new QVBoxLayout(this);
    
    m_queueList = new QListWidget(this);
    layout->addWidget(m_queueList, 1);
    
    m_progressBar = new QProgressBar(this);
    m_progressBar->setVisible(false);
    layout->addWidget(m_progressBar);
    
    m_statusLabel = new QLabel("No items in queue", this);
    layout->addWidget(m_statusLabel);
    
    QHBoxLayout* btnLayout = new QHBoxLayout();
    m_renderBtn = new QPushButton("Render", this);
    m_removeBtn = new QPushButton("Remove", this);
    QPushButton* outputBtn = new QPushButton("Output Module...", this);
    QPushButton* settingsBtn = new QPushButton("Render Settings...", this);
    
    connect(m_renderBtn, &QPushButton::clicked, this, &RenderQueueDialog::onRenderClicked);
    connect(m_removeBtn, &QPushButton::clicked, this, &RenderQueueDialog::onRemoveClicked);
    
    btnLayout->addWidget(m_renderBtn);
    btnLayout->addWidget(m_removeBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(settingsBtn);
    btnLayout->addWidget(outputBtn);
    
    layout->addLayout(btnLayout);
}

void RenderQueueDialog::addItem(const QString& compositionName, const QString& outputPath) {
    RenderQueueItem item;
    item.compositionName = compositionName;
    item.outputPath = outputPath;
    item.status = "Queued";
    m_items.append(item);
    
    m_queueList->addItem(QString("%1 -> %2 [%3]")
        .arg(compositionName, outputPath, item.status));
}

void RenderQueueDialog::updateItemStatus(int index, const QString& status, double progress) {
    if (index < 0 || index >= m_items.size()) return;
    m_items[index].status = status;
    m_items[index].progress = progress;
    
    QListWidgetItem* listItem = m_queueList->item(index);
    if (listItem) {
        listItem->setText(QString("%1 -> %2 [%3]")
            .arg(m_items[index].compositionName,
                 m_items[index].outputPath, status));
    }
}

void RenderQueueDialog::onRenderClicked() {
    if (m_items.isEmpty()) {
        QMessageBox::information(this, "Render Queue", "No items to render.");
        return;
    }
    m_progressBar->setVisible(true);
    m_statusLabel->setText("Rendering...");
}

void RenderQueueDialog::onRemoveClicked() {
    int row = m_queueList->currentRow();
    if (row >= 0) {
        m_queueList->takeItem(row);
        m_items.removeAt(row);
    }
}

} // namespace FreeEffect
