#include "effects_browser_panel.h"
#include "../mainwindow/main_window.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QTreeWidget>
#include <QListWidget>
#include <QTabWidget>
#include <QHeaderView>
#include <QSettings>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QApplication>

namespace FreeEffect {

EffectsBrowserPanel::EffectsBrowserPanel(MainWindow* parent)
    : QWidget(parent)
    , m_mainWindow(parent) {
    setupUi();
    loadFavorites();
    populateEffects();
}

void EffectsBrowserPanel::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    QWidget* searchWidget = new QWidget(this);
    searchWidget->setStyleSheet("background-color: #2a2a2a; border-bottom: 1px solid #1a1a1a;");
    searchWidget->setFixedHeight(32);
    
    QHBoxLayout* searchLayout = new QHBoxLayout(searchWidget);
    searchLayout->setContentsMargins(6, 4, 6, 4);
    searchLayout->setSpacing(4);
    
    m_searchBox = new QLineEdit(searchWidget);
    m_searchBox->setPlaceholderText("Search effects...");
    m_searchBox->setClearButtonEnabled(true);
    m_searchBox->setStyleSheet(
        "QLineEdit { background-color: #383838; color: #cccccc; border: 1px solid #555555; "
        "border-radius: 3px; padding: 3px 8px; font-size: 11px; }"
        "QLineEdit:focus { border: 1px solid #00d4ff; }"
        "QLineEdit::placeholder { color: #777777; }"
    );
    connect(m_searchBox, &QLineEdit::textChanged, this, &EffectsBrowserPanel::onSearchTextChanged);
    
    searchLayout->addWidget(m_searchBox, 1);
    
    mainLayout->addWidget(searchWidget);
    
    setupTabs();
    
    mainLayout->addWidget(m_tabWidget);
}

void EffectsBrowserPanel::setupTabs() {
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setStyleSheet(
        "QTabWidget::pane { border: none; background: #1e1e1e; }"
        "QTabBar { background: #2d2d2d; border-bottom: 1px solid #1a1a1a; }"
        "QTabBar::tab { background: #383838; color: #888888; padding: 6px 12px; "
        "border: none; border-bottom: 2px solid transparent; font-size: 11px; }"
        "QTabBar::tab:selected { color: #00d4ff; border-bottom: 2px solid #00d4ff; background: #2d2d2d; }"
        "QTabBar::tab:hover { color: #cccccc; background: #404040; }"
    );
    
    QWidget* allEffectsTab = new QWidget();
    QVBoxLayout* allLayout = new QVBoxLayout(allEffectsTab);
    allLayout->setContentsMargins(0, 0, 0, 0);
    allLayout->setSpacing(0);
    
    m_categoryTree = new QTreeWidget(allEffectsTab);
    m_categoryTree->setHeaderHidden(true);
    m_categoryTree->setRootIsDecorated(true);
    m_categoryTree->setIndentation(16);
    m_categoryTree->setAnimated(true);
    m_categoryTree->setExpandsOnDoubleClick(true);
    m_categoryTree->setSelectionMode(QAbstractItemView::SingleSelection);
    m_categoryTree->setStyleSheet(
        "QTreeWidget { background: #1e1e1e; color: #cccccc; border: none; "
        "font-size: 11px; outline: none; }"
        "QTreeWidget::item { padding: 3px 4px; border-radius: 2px; }"
        "QTreeWidget::item:selected { background: #2a5a7a; color: #ffffff; }"
        "QTreeWidget::item:hover { background: #333333; }"
        "QTreeWidget::branch { background: #1e1e1e; }"
        "QTreeWidget::branch:has-children:closed { image: none; }"
        "QTreeWidget::branch:has-children:open { image: none; }"
    );
    m_categoryTree->header()->setStretchLastSection(true);
    connect(m_categoryTree, &QTreeWidget::itemDoubleClicked, this, &EffectsBrowserPanel::onEffectDoubleClicked);
    connect(m_categoryTree, &QTreeWidget::itemClicked, this, &EffectsBrowserPanel::onEffectItemClicked);
    
    allLayout->addWidget(m_categoryTree);
    m_tabWidget->addTab(allEffectsTab, "All Effects");
    
    QWidget* favoritesTab = new QWidget();
    QVBoxLayout* favLayout = new QVBoxLayout(favoritesTab);
    favLayout->setContentsMargins(0, 0, 0, 0);
    favLayout->setSpacing(0);
    
    m_favoritesList = new QListWidget(favoritesTab);
    m_favoritesList->setStyleSheet(
        "QListWidget { background: #1e1e1e; color: #cccccc; border: none; "
        "font-size: 11px; outline: none; }"
        "QListWidget::item { padding: 4px 8px; border-radius: 2px; }"
        "QListWidget::item:selected { background: #2a5a7a; color: #ffffff; }"
        "QListWidget::item:hover { background: #333333; }"
    );
    connect(m_favoritesList, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem* item) {
        if (item) applyEffectToLayer(item->text());
    });
    
    favLayout->addWidget(m_favoritesList);
    m_tabWidget->addTab(favoritesTab, "Favorites");
    
    QWidget* recentTab = new QWidget();
    QVBoxLayout* recentLayout = new QVBoxLayout(recentTab);
    recentLayout->setContentsMargins(0, 0, 0, 0);
    recentLayout->setSpacing(0);
    
    m_recentlyUsedList = new QListWidget(recentTab);
    m_recentlyUsedList->setStyleSheet(m_favoritesList->styleSheet());
    connect(m_recentlyUsedList, &QListWidget::itemDoubleClicked, 
            this, &EffectsBrowserPanel::onRecentlyUsedClicked);
    
    recentLayout->addWidget(m_recentlyUsedList);
    m_tabWidget->addTab(recentTab, "Recent");
}

void EffectsBrowserPanel::setupCategoryTree() {
}

void EffectsBrowserPanel::setupFavoritesSection() {
}

void EffectsBrowserPanel::setupRecentlyUsed() {
}

void EffectsBrowserPanel::populateEffects() {
    m_categoryTree->clear();
    m_allEffectNames.clear();
    
    auto& registry = EffectRegistry::instance();
    auto categories = registry.getCategories();
    
    QStringList categoryOrder = {
        "Blur & Sharpen",
        "Color Correction",
        "Distort",
        "Generate",
        "Keying",
        "Perspective",
        "Stylize",
        "Utility"
    };
    
    for (const auto& cat : categoryOrder) {
        auto effects = registry.getEffectNamesInCategory(cat.toStdString());
        if (effects.empty()) continue;
        
        QTreeWidgetItem* catItem = new QTreeWidgetItem(m_categoryTree);
        catItem->setText(0, cat);
        catItem->setFlags(catItem->flags() & ~Qt::ItemIsSelectable);
        
        QFont catFont = catItem->font(0);
        catFont.setBold(true);
        catFont.setPointSize(11);
        catItem->setFont(0, catFont);
        catItem->setForeground(0, QColor(0, 212, 255));
        
        for (const auto& eff : effects) {
            QString effName = QString::fromStdString(eff);
            m_allEffectNames.append(effName);
            
            QTreeWidgetItem* effItem = new QTreeWidgetItem(catItem);
            effItem->setText(0, effName);
            effItem->setToolTip(0, effName);
            
            if (m_favorites.contains(effName)) {
                effItem->setForeground(0, QColor(255, 200, 50));
            }
        }
        
        catItem->setExpanded(true);
    }
    
    QStringList uncategorized;
    for (const auto& info : registry.getAllEffects()) {
        QString name = QString::fromStdString(info.name);
        if (!m_allEffectNames.contains(name)) {
            m_allEffectNames.append(name);
            uncategorized.append(name);
        }
    }
    
    if (!uncategorized.empty()) {
        QTreeWidgetItem* catItem = new QTreeWidgetItem(m_categoryTree);
        catItem->setText(0, "Other");
        catItem->setFlags(catItem->flags() & ~Qt::ItemIsSelectable);
        
        QFont catFont = catItem->font(0);
        catFont.setBold(true);
        catItem->setFont(0, catFont);
        catItem->setForeground(0, QColor(0, 212, 255));
        
        for (const auto& name : uncategorized) {
            QTreeWidgetItem* effItem = new QTreeWidgetItem(catItem);
            effItem->setText(0, name);
        }
        catItem->setExpanded(true);
    }
    
    m_categoryTree->sortByColumn(0, Qt::AscendingOrder);
}

void EffectsBrowserPanel::filterEffects(const QString& filter) {
    if (filter.isEmpty()) {
        for (int i = 0; i < m_categoryTree->topLevelItemCount(); i++) {
            QTreeWidgetItem* catItem = m_categoryTree->topLevelItem(i);
            catItem->setHidden(false);
            for (int j = 0; j < catItem->childCount(); j++) {
                catItem->child(j)->setHidden(false);
            }
        }
        return;
    }
    
    QString lowerFilter = filter.toLower();
    
    for (int i = 0; i < m_categoryTree->topLevelItemCount(); i++) {
        QTreeWidgetItem* catItem = m_categoryTree->topLevelItem(i);
        bool hasVisibleChild = false;
        
        for (int j = 0; j < catItem->childCount(); j++) {
            QTreeWidgetItem* effItem = catItem->child(j);
            bool match = effItem->text(0).toLower().contains(lowerFilter);
            effItem->setHidden(!match);
            if (match) hasVisibleChild = true;
        }
        
        catItem->setHidden(!hasVisibleChild);
        if (hasVisibleChild) {
            catItem->setExpanded(true);
        }
    }
}

void EffectsBrowserPanel::onSearchTextChanged(const QString& text) {
    filterEffects(text);
}

void EffectsBrowserPanel::onEffectDoubleClicked(QTreeWidgetItem* item, int column) {
    if (!item || item->childCount() > 0) return;
    applyEffectToLayer(item->text(0));
}

void EffectsBrowserPanel::onEffectItemClicked(QTreeWidgetItem* item, int column) {
}

void EffectsBrowserPanel::onFavoriteToggled(QTreeWidgetItem* item, int column) {
}

void EffectsBrowserPanel::onRecentlyUsedClicked(QListWidgetItem* item) {
    if (item) applyEffectToLayer(item->text());
}

void EffectsBrowserPanel::applyEffectToLayer(const QString& effectName) {
    if (effectName.isEmpty()) return;
    
    addToRecentlyUsed(effectName);
    emit effectRequested(effectName);
}

void EffectsBrowserPanel::addToRecentlyUsed(const QString& effectName) {
    m_recentlyUsed.removeAll(effectName);
    m_recentlyUsed.prepend(effectName);
    
    while (m_recentlyUsed.size() > kMaxRecentlyUsed) {
        m_recentlyUsed.removeLast();
    }
    
    m_recentlyUsedList->clear();
    for (const auto& name : m_recentlyUsed) {
        m_recentlyUsedList->addItem(name);
    }
    
    QSettings settings("FreeEffect", "FreeEffect");
    settings.setValue("recentlyUsedEffects", m_recentlyUsed);
}

void EffectsBrowserPanel::loadFavorites() {
    QSettings settings("FreeEffect", "FreeEffect");
    m_recentlyUsed = settings.value("recentlyUsedEffects").toStringList();
    
    QJsonArray favArray = QJsonDocument::fromJson(
        settings.value("favoriteEffects").toString().toUtf8()).array();
    for (const auto& val : favArray) {
        m_favorites.insert(val.toString());
    }
    
    m_recentlyUsedList->clear();
    for (const auto& name : m_recentlyUsed) {
        m_recentlyUsedList->addItem(name);
    }
}

void EffectsBrowserPanel::saveFavorites() {
    QSettings settings("FreeEffect", "FreeEffect");
    
    QJsonArray favArray;
    for (const auto& fav : m_favorites) {
        favArray.append(fav);
    }
    settings.setValue("favoriteEffects", 
        QString(QJsonDocument(favArray).toJson()));
}

void EffectsBrowserPanel::refreshEffects() {
    populateEffects();
}

void EffectsBrowserPanel::styleTreeWidget(QTreeWidget* tree) {
}

} // namespace FreeEffect
