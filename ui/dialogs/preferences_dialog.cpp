#include "preferences_dialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QDialogButtonBox>

namespace FreeEffect {

PreferencesDialog::PreferencesDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle("Preferences");
    setMinimumSize(550, 450);
    setupUi();
}

void PreferencesDialog::setupUi() {
    QVBoxLayout* layout = new QVBoxLayout(this);
    
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->addTab(createGeneralTab(), "General");
    m_tabWidget->addTab(createImportTab(), "Import");
    m_tabWidget->addTab(createOutputTab(), "Output");
    m_tabWidget->addTab(createGridsGuidesTab(), "Grids & Guides");
    m_tabWidget->addTab(createLabelsTab(), "Labels");
    m_tabWidget->addTab(createMediaCacheTab(), "Media & Disk Cache");
    m_tabWidget->addTab(createMemoryCpuTab(), "Memory & CPU");
    m_tabWidget->addTab(createAudioHardwareTab(), "Audio Hardware");
    
    layout->addWidget(m_tabWidget);
    
    QDialogButtonBox* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}

QWidget* PreferencesDialog::createGeneralTab() {
    QWidget* tab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(tab);
    
    QGridLayout* grid = new QGridLayout();
    grid->addWidget(new QLabel("Levels of Undo:", tab), 0, 0);
    QSpinBox* undoSpin = new QSpinBox(tab);
    undoSpin->setRange(1, 100);
    undoSpin->setValue(32);
    grid->addWidget(undoSpin, 0, 1);
    
    layout->addLayout(grid);
    layout->addStretch();
    return tab;
}

QWidget* PreferencesDialog::createImportTab() {
    QWidget* tab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(tab);
    layout->addWidget(new QLabel("Import Settings", tab));
    layout->addStretch();
    return tab;
}

QWidget* PreferencesDialog::createOutputTab() {
    QWidget* tab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(tab);
    layout->addWidget(new QLabel("Output Settings", tab));
    layout->addStretch();
    return tab;
}

QWidget* PreferencesDialog::createGridsGuidesTab() {
    QWidget* tab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(tab);
    layout->addWidget(new QLabel("Grids & Guides Settings", tab));
    layout->addStretch();
    return tab;
}

QWidget* PreferencesDialog::createLabelsTab() {
    QWidget* tab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(tab);
    layout->addWidget(new QLabel("Label Colors", tab));
    layout->addStretch();
    return tab;
}

QWidget* PreferencesDialog::createMediaCacheTab() {
    QWidget* tab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(tab);
    
    layout->addWidget(new QLabel("Media Cache Location:", tab));
    QHBoxLayout* cachePath = new QHBoxLayout();
    QLineEdit* pathEdit = new QLineEdit(tab);
    QPushButton* browseBtn = new QPushButton("Browse...", tab);
    cachePath->addWidget(pathEdit);
    cachePath->addWidget(browseBtn);
    layout->addLayout(cachePath);
    
    QPushButton* emptyCacheBtn = new QPushButton("Empty Disk Cache", tab);
    layout->addWidget(emptyCacheBtn);
    layout->addStretch();
    return tab;
}

QWidget* PreferencesDialog::createMemoryCpuTab() {
    QWidget* tab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(tab);
    layout->addWidget(new QLabel("Memory & CPU Settings", tab));
    layout->addStretch();
    return tab;
}

QWidget* PreferencesDialog::createAudioHardwareTab() {
    QWidget* tab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(tab);
    layout->addWidget(new QLabel("Audio Hardware Settings", tab));
    layout->addStretch();
    return tab;
}

} // namespace FreeEffect
