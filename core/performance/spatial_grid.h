#pragma once
#include <vector>
#include <functional>
#include <cstdint>

namespace FreeEffect {

struct SpatialItem {
    int layerIndex = -1;
    float bounds[4]; // x, y, width, height
    int zOrder = 0;
};

class SpatialGrid {
public:
    void setBounds(float x, float y, float w, float h, int cellSize = 64);
    void clear();
    void insert(const SpatialItem& item);

    std::vector<SpatialItem> queryPoint(float x, float y) const;
    std::vector<SpatialItem> queryRect(float x, float y, float w, float h) const;

    SpatialItem* findTopmost(float x, float y);

    void sortByZOrder();

    int getItemCount() const { return static_cast<int>(m_items.size()); }
    const SpatialItem& getItem(int index) const { return m_items[index]; }

private:
    int m_cellSize = 64;
    float m_originX = 0, m_originY = 0;
    float m_width = 0, m_height = 0;
    int m_cols = 0, m_rows = 0;

    struct Cell {
        std::vector<int> itemIndices;
    };
    std::vector<Cell> m_grid;
    std::vector<SpatialItem> m_items;

    int cellIndex(int col, int row) const;
    int getCol(float x) const;
    int getRow(float y) const;
    bool boundsOverlap(const SpatialItem& a, const SpatialItem& b) const;
    bool pointInItem(float px, float py, const SpatialItem& item) const;
    bool rectsOverlap(float ax, float ay, float aw, float ah,
                      float bx, float by, float bw, float bh) const;
};

} // namespace FreeEffect
