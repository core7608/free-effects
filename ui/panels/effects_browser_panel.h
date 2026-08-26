#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QTreeWidget>
#include <QLabel>
#include <QListWidget>
#include <QSplitter>
#include <QTabWidget>
#include <QSet>
#include <QStringList>
#include "../../core/effects/effect_registry.h"

namespace FreeEffect {

class MainWindow;

class EffectsBrowserPanel : public QWidget {
    Q_OBJECT
public:
    explicit EffectsBrowserPanel(MainWindow* parent);
    ~EffectsBrowserPanel() override = default;
    
    void refreshEffects();
    void applyEffectToLayer(const QString& effectName);
    void setSelectedLayerIndex(int index) { m_selectedLayerIndex = index; }

signals:
    void effectRequested(const QString& effectName);

private slots:
    void onSearchTextChanged(const QString& text);
    void onEffectDoubleClicked(QTreeWidgetItem* item, int column);
    void onEffectItemClicked(QTreeWidgetItem* item, int column);
    void onFavoriteToggled(QTreeWidgetItem* item, int column);
    void onRecentlyUsedClicked(QListWidgetItem* item);

private:
    void setupUi();
    void setupTabs();
    void setupCategoryTree();
    void setupFavoritesSection();
    void setupRecentlyUsed();
    void populateEffects();
    void filterEffects(const QString& filter);
    void addToRecentlyUsed(const QString& effectName);
    void loadFavorites();
    void saveFavorites();
    void styleTreeWidget(QTreeWidget* tree);
    
    MainWindow* m_mainWindow;
    int m_selectedLayerIndex = -1;
    
    QLineEdit* m_searchBox = nullptr;
    QTabWidget* m_tabWidget = nullptr;
    QTreeWidget* m_categoryTree = nullptr;
    QListWidget* m_favoritesList = nullptr;
    QListWidget* m_recentlyUsedList = nullptr;
    
    QSet<QString> m_favorites;
    QStringList m_recentlyUsed;
    QStringList m_allEffectNames;
    
    static constexpr int kMaxRecentlyUsed = 15;
};

} // namespace FreeEffect
