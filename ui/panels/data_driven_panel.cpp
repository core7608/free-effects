#include "data_driven_panel.h"
#include "../mainwindow/main_window.h"
#include <QGridLayout>
#include <QFrame>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

namespace FreeEffect {

DataDrivenPanel::DataDrivenPanel(MainWindow* parent)
    : QWidget(parent)
    , m_mainWindow(parent) {
    setupUi();
}

void DataDrivenPanel::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(6);

    QGroupBox* importGroup = new QGroupBox("Data Source", this);
    QVBoxLayout* iLayout = new QVBoxLayout(importGroup);

    m_importBtn = new QPushButton("Import JSON...", this);
    iLayout->addWidget(m_importBtn);
    connect(m_importBtn, &QPushButton::clicked, this, &DataDrivenPanel::onImportJSON);

    mainLayout->addWidget(importGroup);

    QGroupBox* treeGroup = new QGroupBox("Data Tree", this);
    QVBoxLayout* tLayout = new QVBoxLayout(treeGroup);

    m_dataTree = new QTreeWidget(this);
    m_dataTree->setHeaderLabels({"Key", "Value", "Type"});
    m_dataTree->setColumnWidth(0, 150);
    m_dataTree->setColumnWidth(1, 120);
    m_dataTree->setColumnWidth(2, 80);
    tLayout->addWidget(m_dataTree);

    connect(m_dataTree, &QTreeWidget::itemClicked, this, &DataDrivenPanel::onTreeItemClicked);

    m_refreshBtn = new QPushButton("Refresh", this);
    tLayout->addWidget(m_refreshBtn);
    connect(m_refreshBtn, &QPushButton::clicked, this, &DataDrivenPanel::onRefreshData);

    mainLayout->addWidget(treeGroup);

    QFrame* sep = new QFrame(this);
    sep->setFrameShape(QFrame::HLine);
    sep->setFrameShadow(QFrame::Sunken);
    mainLayout->addWidget(sep);

    QGroupBox* bindGroup = new QGroupBox("Bind Property", this);
    QGridLayout* bLayout = new QGridLayout(bindGroup);

    bLayout->addWidget(new QLabel("Data Path:", this), 0, 0);
    m_dataPathEdit = new QLineEdit(this);
    m_dataPathEdit->setPlaceholderText("e.g. data.position.x");
    bLayout->addWidget(m_dataPathEdit, 0, 1);

    bLayout->addWidget(new QLabel("Property:", this), 1, 0);
    m_propertyPathEdit = new QLineEdit(this);
    m_propertyPathEdit->setPlaceholderText("e.g. Transform.Position");
    bLayout->addWidget(m_propertyPathEdit, 1, 1);

    m_bindBtn = new QPushButton("Bind", this);
    bLayout->addWidget(m_bindBtn, 2, 0, 1, 2);
    connect(m_bindBtn, &QPushButton::clicked, this, &DataDrivenPanel::onBindProperty);

    mainLayout->addWidget(bindGroup);

    m_previewLabel = new QLabel("Preview: --", this);
    m_previewLabel->setStyleSheet("color: #888888; font-size: 11px;");
    mainLayout->addWidget(m_previewLabel);

    mainLayout->addStretch();
}

void DataDrivenPanel::onImportJSON() {
    QString file = QFileDialog::getOpenFileName(this, "Import JSON", QString(),
        "JSON Files (*.json);;All Files (*)");
    if (file.isEmpty()) return;

    QFile f(file);
    if (!f.open(QIODevice::ReadOnly)) return;
    QTextStream stream(&f);
    QString content = stream.readAll();
    f.close();

    populateTree(content);
}

void DataDrivenPanel::populateTree(const QString& jsonStr) {
    m_dataTree->clear();

    QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
    if (doc.isNull()) return;

    std::function<void(QJsonValue, QTreeWidgetItem*)> addValue =
        [&](QJsonValue val, QTreeWidgetItem* parent) {
        if (val.isObject()) {
            QJsonObject obj = val.toObject();
            for (auto it = obj.begin(); it != obj.end(); ++it) {
                QTreeWidgetItem* item = new QTreeWidgetItem(parent);
                item->setText(0, it.key());
                if (it.value().isObject() || it.value().isArray()) {
                    item->setText(2, it.value().isObject() ? "object" : "array");
                    addValue(it.value(), item);
                } else {
                    item->setText(1, it.value().toVariant().toString());
                    item->setText(2, it.value().isDouble() ? "number" :
                                    it.value().isBool() ? "boolean" : "string");
                }
            }
        } else if (val.isArray()) {
            QJsonArray arr = val.toArray();
            for (int i = 0; i < arr.size(); ++i) {
                QTreeWidgetItem* item = new QTreeWidgetItem(parent);
                item->setText(0, QString("[%1]").arg(i));
                addValue(arr[i], item);
            }
        }
    };

    if (doc.isObject()) {
        addValue(doc.object(), m_dataTree->invisibleRootItem());
    } else if (doc.isArray()) {
        addValue(doc.array(), m_dataTree->invisibleRootItem());
    }

    m_dataTree->expandAll();
}

void DataDrivenPanel::onBindProperty() {
    emit bindPropertyRequested(m_dataPathEdit->text(), m_propertyPathEdit->text());
}

void DataDrivenPanel::onRefreshData() { emit refreshDataRequested(); }

void DataDrivenPanel::onTreeItemClicked(QTreeWidgetItem* item, int column) {
    if (item) {
        m_dataPathEdit->setText(item->text(0));
        m_previewLabel->setText("Preview: " + item->text(1));
    }
}

} // namespace FreeEffect
