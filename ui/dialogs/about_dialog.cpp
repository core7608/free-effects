#include "about_dialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

namespace FreeEffect {

AboutDialog::AboutDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle("About FreeEffect");
    setFixedSize(400, 300);
    setStyleSheet("background-color: #0e0e0e;");
    setupUi();
}

void AboutDialog::setupUi() {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(8);
    
    QLabel* logoLabel = new QLabel("FE", this);
    logoLabel->setAlignment(Qt::AlignCenter);
    logoLabel->setStyleSheet("font-size: 48px; font-weight: 900; color: #ffffff;");
    layout->addWidget(logoLabel);
    
    QLabel* titleLabel = new QLabel("FreeEffect", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: 700; color: #ffffff;");
    layout->addWidget(titleLabel);
    
    QLabel* versionLabel = new QLabel("Version 0.1.0 (Alpha)", this);
    versionLabel->setAlignment(Qt::AlignCenter);
    versionLabel->setStyleSheet("color: #555555; font-size: 11px;");
    layout->addWidget(versionLabel);
    
    layout->addSpacing(12);
    
    QLabel* descLabel = new QLabel(
        "A free and open-source alternative to Adobe After Effects.\n\n"
        "Licensed under the GNU General Public License v3.0.\n"
        "Built with Qt 6 and C++17.", this);
    descLabel->setAlignment(Qt::AlignCenter);
    descLabel->setWordWrap(true);
    descLabel->setStyleSheet("color: #888888; font-size: 11px;");
    layout->addWidget(descLabel);
    
    layout->addSpacing(12);
    
    QLabel* linkLabel = new QLabel(
        "<a href=\"https://github.com/FreeEffect/FreeEffect\" style=\"color: #ffffff; text-decoration: none;\">GitHub Repository</a>", this);
    linkLabel->setAlignment(Qt::AlignCenter);
    linkLabel->setOpenExternalLinks(true);
    layout->addWidget(linkLabel);
    
    layout->addSpacing(16);
    
    QPushButton* okBtn = new QPushButton("OK", this);
    okBtn->setFixedWidth(80);
    connect(okBtn, &QPushButton::clicked, this, &QDialog::accept);
    layout->addWidget(okBtn, 0, Qt::AlignCenter);
}

} // namespace FreeEffect
