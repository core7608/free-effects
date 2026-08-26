#include "spatial_grid.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace FreeEffect {

void SpatialGrid::setBounds(float x, float y, float w, float h, int cellSize) {
    m_originX = x;
    m_originY = y;
    m_width = w;
    m_height = h;
    m_cellSize = cellSize;
    m_cols = std::max(1, static_cast<int>(std::ceil(w / cellSize)));
    m_rows = std::max(1, static_cast<int>(std::ceil(h / cellSize)));
    m_grid.resize(m_cols * m_rows);
}

void SpatialGrid::clear() {
    m_items.clear();
    for (auto& cell : m_grid) {
        cell.itemIndices.clear();
    }
}

void SpatialGrid::insert(const SpatialItem& item) {
    int index = static_cast<int>(m_items.size());
    m_items.push_back(item);

    if (m_grid.empty()) return;

    int minCol = getCol(item.bounds[0]);
    int maxCol = getCol(item.bounds[0] + item.bounds[2]);
    int minRow = getRow(item.bounds[1]);
    int maxRow = getRow(item.bounds[1] + item.bounds[3]);

    minCol = std::max(0, minCol);
    maxCol = std::min(m_cols - 1, maxCol);
    minRow = std::max(0, minRow);
    maxRow = std::min(m_rows - 1, maxRow);

    for (int r = minRow; r <= maxRow; ++r) {
        for (int c = minCol; c <= maxCol; ++c) {
            m_grid[cellIndex(c, r)].itemIndices.push_back(index);
        }
    }
}

std::vector<SpatialItem> SpatialGrid::queryPoint(float x, float y) const {
    std::vector<SpatialItem> result;
    if (m_grid.empty()) return result;

    int col = getCol(x);
    int row = getRow(y);
    if (col < 0 || col >= m_cols || row < 0 || row >= m_rows) return result;

    const Cell& cell = m_grid[cellIndex(col, row)];
    for (int idx : cell.itemIndices) {
        if (pointInItem(x, y, m_items[idx])) {
            result.push_back(m_items[idx]);
        }
    }
    return result;
}

std::vector<SpatialItem> SpatialGrid::queryRect(float x, float y, float w, float h) const {
    std::vector<SpatialItem> result;
    if (m_grid.empty()) return result;

    int minCol = getCol(x);
    int maxCol = getCol(x + w);
    int minRow = getRow(y);
    int maxRow = getRow(y + h);

    minCol = std::max(0, minCol);
    maxCol = std::min(m_cols - 1, maxCol);
    minRow = std::max(0, minRow);
    maxRow = std::min(m_rows - 1, maxRow);

    std::vector<bool> seen(m_items.size(), false);

    for (int r = minRow; r <= maxRow; ++r) {
        for (int c = minCol; c <= maxCol; ++c) {
            const Cell& cell = m_grid[cellIndex(c, r)];
            for (int idx : cell.itemIndices) {
                if (seen[idx]) continue;
                seen[idx] = true;
                const SpatialItem& item = m_items[idx];
                if (rectsOverlap(x, y, w, h,
                                 item.bounds[0], item.bounds[1], item.bounds[2], item.bounds[3])) {
                    result.push_back(item);
                }
            }
        }
    }
    return result;
}

SpatialItem* SpatialGrid::findTopmost(float x, float y) {
    SpatialItem* topmost = nullptr;
    int highestZ = std::numeric_limits<int>::min();

    if (m_grid.empty()) return nullptr;

    int col = getCol(x);
    int row = getRow(y);
    if (col < 0 || col >= m_cols || row < 0 || row >= m_rows) return nullptr;

    const Cell& cell = m_grid[cellIndex(col, row)];
    for (int idx : cell.itemIndices) {
        SpatialItem& item = m_items[idx];
        if (pointInItem(x, y, item) && item.zOrder > highestZ) {
            highestZ = item.zOrder;
            topmost = &item;
        }
    }
    return topmost;
}

void SpatialGrid::sortByZOrder() {
    std::sort(m_items.begin(), m_items.end(),
        [](const SpatialItem& a, const SpatialItem& b) {
            return a.zOrder < b.zOrder;
        });

    for (auto& cell : m_grid) {
        cell.itemIndices.clear();
    }

    for (int i = 0; i < static_cast<int>(m_items.size()); ++i) {
        const SpatialItem& item = m_items[i];
        int minCol = getCol(item.bounds[0]);
        int maxCol = getCol(item.bounds[0] + item.bounds[2]);
        int minRow = getRow(item.bounds[1]);
        int maxRow = getRow(item.bounds[1] + item.bounds[3]);

        minCol = std::max(0, minCol);
        maxCol = std::min(m_cols - 1, maxCol);
        minRow = std::max(0, minRow);
        maxRow = std::min(m_rows - 1, maxRow);

        for (int r = minRow; r <= maxRow; ++r) {
            for (int c = minCol; c <= maxCol; ++c) {
                m_grid[cellIndex(c, r)].itemIndices.push_back(i);
            }
        }
    }
}

int SpatialGrid::cellIndex(int col, int row) const {
    return row * m_cols + col;
}

int SpatialGrid::getCol(float x) const {
    return static_cast<int>(std::floor((x - m_originX) / m_cellSize));
}

int SpatialGrid::getRow(float y) const {
    return static_cast<int>(std::floor((y - m_originY) / m_cellSize));
}

bool SpatialGrid::boundsOverlap(const SpatialItem& a, const SpatialItem& b) const {
    return rectsOverlap(a.bounds[0], a.bounds[1], a.bounds[2], a.bounds[3],
                        b.bounds[0], b.bounds[1], b.bounds[2], b.bounds[3]);
}

bool SpatialGrid::pointInItem(float px, float py, const SpatialItem& item) const {
    return px >= item.bounds[0] && px <= item.bounds[0] + item.bounds[2] &&
           py >= item.bounds[1] && py <= item.bounds[1] + item.bounds[3];
}

bool SpatialGrid::rectsOverlap(float ax, float ay, float aw, float ah,
                                float bx, float by, float bw, float bh) const {
    return ax < bx + bw && ax + aw > bx &&
           ay < by + bh && ay + ah > by;
}

} // namespace FreeEffect
