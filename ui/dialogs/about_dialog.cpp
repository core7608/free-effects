#include "about_dialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

namespace FreeEffect {

AboutDialog::AboutDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle("About FreeEffect");
    setFixedSize(400, 300);
    setupUi();
}

void AboutDialog::setupUi() {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);
    
    QLabel* titleLabel = new QLabel("<h1>FreeEffect</h1>", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel);
    
    QLabel* versionLabel = new QLabel("Version 0.1.0 (Alpha)", this);
    versionLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(versionLabel);
    
    layout->addSpacing(10);
    
    QLabel* descLabel = new QLabel(
        "A free and open-source alternative to Adobe After Effects.\n\n"
        "Licensed under the GNU General Public License v3.0.\n\n"
        "Built with Qt 6 and C++17.", this);
    descLabel->setAlignment(Qt::AlignCenter);
    descLabel->setWordWrap(true);
    layout->addWidget(descLabel);
    
    layout->addSpacing(10);
    
    QLabel* linkLabel = new QLabel(
        "<a href=\"https://github.com/yourusername/FreeEffect\">GitHub Repository</a>", this);
    linkLabel->setAlignment(Qt::AlignCenter);
    linkLabel->setOpenExternalLinks(true);
    layout->addWidget(linkLabel);
    
    layout->addSpacing(10);
    
    QPushButton* okBtn = new QPushButton("OK", this);
    connect(okBtn, &QPushButton::clicked, this, &QDialog::accept);
    layout->addWidget(okBtn, 0, Qt::AlignCenter);
}

} // namespace FreeEffect
