#include "missing_footage_dialog.h"
#include "../mainwindow/main_window.h"
#include <QFileDialog>
#include <QMessageBox>

namespace FreeEffect {

MissingFootageDialog::MissingFootageDialog(const std::vector<MissingFootage>& missing, MainWindow* parent)
    : QDialog(parent)
    , m_mainWindow(parent)
    , m_missingItems(missing) {
    setWindowTitle("Missing Footage");
    setMinimumSize(500, 350);
    setupUi();
}

void MissingFootageDialog::setupUi() {
    QVBoxLayout* layout = new QVBoxLayout(this);
    
    QLabel* infoLabel = new QLabel(
        "The following files are missing from the project. "
        "Use Locate to find them at their new location.", this);
    infoLabel->setWordWrap(true);
    layout->addWidget(infoLabel);
    
    m_listWidget = new QListWidget(this);
    layout->addWidget(m_listWidget, 1);
    
    QHBoxLayout* btnLayout = new QHBoxLayout();
    m_relinkBtn = new QPushButton("Locate...", this);
    m_relinkAllBtn = new QPushButton("Relink All...", this);
    QPushButton* skipBtn = new QPushButton("Skip", this);
    
    connect(m_relinkBtn, &QPushButton::clicked, this, &MissingFootageDialog::onRelinkClicked);
    connect(m_relinkAllBtn, &QPushButton::clicked, this, &MissingFootageDialog::onRelinkAllClicked);
    connect(skipBtn, &QPushButton::clicked, this, &QDialog::reject);
    
    btnLayout->addWidget(m_relinkBtn);
    btnLayout->addWidget(m_relinkAllBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(skipBtn);
    
    layout->addLayout(btnLayout);
    
    refreshList();
}

void MissingFootageDialog::refreshList() {
    m_listWidget->clear();
    for (const auto& item : m_missingItems) {
        m_listWidget->addItem(QString("%1 (%2)")
            .arg(QString::fromStdString(item.name),
                 QString::fromStdString(item.expectedPath)));
    }
}

void MissingFootageDialog::onRelinkClicked() {
    int row = m_listWidget->currentRow();
    if (row < 0 || row >= static_cast<int>(m_missingItems.size())) return;
    
    QString file = QFileDialog::getOpenFileName(this,
        "Locate " + QString::fromStdString(m_missingItems[row].name));
    
    if (!file.isEmpty()) {
        MissingFootageHandler handler(&m_mainWindow->getProjectState());
        if (handler.relinkAsset(m_missingItems[row].assetId, file.toStdString())) {
            m_missingItems.erase(m_missingItems.begin() + row);
            refreshList();
            m_relinked = true;
            
            if (m_missingItems.empty()) accept();
        }
    }
}

void MissingFootageDialog::onRelinkAllClicked() {
    QString dir = QFileDialog::getExistingDirectory(this, "Select folder containing missing files");
    if (dir.isEmpty()) return;
    
    MissingFootageHandler handler(&m_mainWindow->getProjectState());
    int relinked = handler.relinkAll("", dir.toStdString());
    
    if (relinked > 0) {
        m_relinked = true;
        QMessageBox::information(this, "Relink", QString("Relinked %1 files.").arg(relinked));
        accept();
    }
}

} // namespace FreeEffect
