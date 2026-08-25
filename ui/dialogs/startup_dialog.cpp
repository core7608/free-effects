#include "startup_dialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QSettings>

namespace FreeEffect {

StartupDialog::StartupDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle("FreeEffect");
    setFixedSize(580, 420);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setupUi();
    populateRecentProjects();
}

void StartupDialog::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    
    // Header
    QWidget* header = new QWidget(this);
    header->setFixedHeight(140);
    header->setStyleSheet("background-color: #000000;");
    
    QVBoxLayout* headerLayout = new QVBoxLayout(header);
    headerLayout->setContentsMargins(30, 20, 30, 15);
    
    QLabel* logoText = new QLabel("FE", header);
    logoText->setStyleSheet("font-size: 40px; font-weight: 900; color: #ffffff; background: transparent;");
    headerLayout->addWidget(logoText);
    
    QLabel* titleText = new QLabel("FreeEffect", header);
    titleText->setStyleSheet("font-size: 22px; font-weight: 700; color: #ffffff; background: transparent;");
    headerLayout->addWidget(titleText);
    
    QLabel* subtitleText = new QLabel("Open Source Motion Graphics & Visual Effects", header);
    subtitleText->setStyleSheet("font-size: 11px; color: #555555; background: transparent;");
    headerLayout->addWidget(subtitleText);
    
    mainLayout->addWidget(header);
    
    // Content
    QWidget* content = new QWidget(this);
    content->setStyleSheet("background-color: #0e0e0e;");
    QVBoxLayout* contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(30, 20, 30, 20);
    contentLayout->setSpacing(10);
    
    // Buttons
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(10);
    
    m_newBtn = new QPushButton("New Project", content);
    m_newBtn->setMinimumHeight(40);
    m_newBtn->setStyleSheet(
        "QPushButton { background-color: #ffffff; color: #000000; border: none; border-radius: 2px; "
        "font-size: 13px; font-weight: bold; padding: 8px 20px; }"
        "QPushButton:hover { background-color: #e0e0e0; }"
        "QPushButton:pressed { background-color: #cccccc; }"
    );
    connect(m_newBtn, &QPushButton::clicked, this, &StartupDialog::onNewProject);
    btnLayout->addWidget(m_newBtn);
    
    m_openBtn = new QPushButton("Open Project", content);
    m_openBtn->setMinimumHeight(40);
    m_openBtn->setStyleSheet(
        "QPushButton { background-color: #1a1a1a; color: #cccccc; border: 1px solid #2a2a2a; "
        "border-radius: 2px; font-size: 13px; padding: 8px 20px; }"
        "QPushButton:hover { background-color: #252525; border-color: #444444; }"
        "QPushButton:pressed { background-color: #ffffff; color: #000000; }"
    );
    connect(m_openBtn, &QPushButton::clicked, this, &StartupDialog::onOpenProject);
    btnLayout->addWidget(m_openBtn);
    
    contentLayout->addLayout(btnLayout);
    
    // Recent projects
    QLabel* recentLabel = new QLabel("Recent Projects", content);
    recentLabel->setStyleSheet("font-size: 11px; color: #555555; font-weight: bold; margin-top: 10px;");
    contentLayout->addWidget(recentLabel);
    
    m_recentList = new QListWidget(content);
    m_recentList->setMaximumHeight(160);
    m_recentList->setStyleSheet(
        "QListWidget { background-color: #141414; color: #cccccc; border: 1px solid #1e1e1e; "
        "border-radius: 2px; font-size: 11px; }"
        "QListWidget::item { padding: 8px; border: none; }"
        "QListWidget::item:selected { background-color: #ffffff; color: #000000; }"
        "QListWidget::item:hover { background-color: #1a1a1a; }"
    );
    connect(m_recentList, &QListWidget::itemDoubleClicked, this, &StartupDialog::onRecentDoubleClicked);
    contentLayout->addWidget(m_recentList, 1);
    
    m_closeBtn = new QPushButton("Close", content);
    m_closeBtn->setFixedWidth(80);
    connect(m_closeBtn, &QPushButton::clicked, this, &StartupDialog::onClose);
    
    QHBoxLayout* closeLayout = new QHBoxLayout();
    closeLayout->addStretch();
    closeLayout->addWidget(m_closeBtn);
    contentLayout->addLayout(closeLayout);
    
    mainLayout->addWidget(content, 1);
}

void StartupDialog::populateRecentProjects() {
    QSettings settings("FreeEffect", "FreeEffect");
    QStringList recent = settings.value("recentProjects").toStringList();
    
    m_recentList->clear();
    for (const QString& path : recent) {
        QListWidgetItem* item = new QListWidgetItem(path, m_recentList);
        item->setData(Qt::UserRole, path);
    }
    
    if (recent.isEmpty()) {
        QListWidgetItem* item = new QListWidgetItem("No recent projects", m_recentList);
        item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
        item->setForeground(QColor(60, 60, 60));
    }
}

void StartupDialog::onNewProject() {
    m_result = NewProject;
    accept();
}

void StartupDialog::onOpenProject() {
    m_result = OpenProject;
    accept();
}

void StartupDialog::onRecentDoubleClicked(QListWidgetItem* item) {
    QString path = item->data(Qt::UserRole).toString();
    if (!path.isEmpty()) {
        m_selectedRecent = path;
        m_result = OpenRecent;
        accept();
    }
}

void StartupDialog::onClose() {
    m_result = Cancel;
    reject();
}

} // namespace FreeEffect
