/**
 * @file FenwickTree6.cpp
 * @brief FenwickTree6 实现
 *
 * 实现二维树状数组：范围求和查询、坐标压缩、稀疏网格支持。
 */

#include "utils/tree212/FenwickTree6.h"

#include <QElapsedTimer>
#include <algorithm>

/* ---- Construction / Destruction ---- */

FenwickTree6::FenwickTree6(QObject *parent) : QObject(parent) {}
FenwickTree6::~FenwickTree6() = default;

/* ---- Initialize fixed grid ---- */

void FenwickTree6::init(int sizeX, int sizeY)
{
    m_sizeX = qMax(1, sizeX);
    m_sizeY = qMax(1, sizeY);

    // BIT is 1-indexed, so allocate size+1
    m_tree.resize(m_sizeX + 1);
    for (auto& row : m_tree)
        row.fill(0.0, m_sizeY + 1);

    m_stats.gridSizeX = m_sizeX;
    m_stats.gridSizeY = m_sizeY;
    m_stats.gridSize = m_sizeX * m_sizeY;
}

/* ---- Initialize with coordinate compression ---- */

void FenwickTree6::initCompressed(const QVector<QPair<int, int>>& points)
{
    if (points.isEmpty()) return;

    // Extract unique X and Y coordinates
    QVector<int> xCoords, yCoords;
    for (const auto& p : points) {
        xCoords.append(p.first);
        yCoords.append(p.second);
    }

    std::sort(xCoords.begin(), xCoords.end());
    xCoords.erase(std::unique(xCoords.begin(), xCoords.end()), xCoords.end());
    m_compressX = xCoords;

    std::sort(yCoords.begin(), yCoords.end());
    yCoords.erase(std::unique(yCoords.begin(), yCoords.end()), yCoords.end());
    m_compressY = yCoords;

    // Init grid with compressed sizes
    init(xCoords.size(), yCoords.size());

    m_stats.compressedX = xCoords.size();
    m_stats.compressedY = yCoords.size();
}

/* ---- Coordinate compression ---- */

int FenwickTree6::compressX(int x) const
{
    if (m_compressX.isEmpty()) return x;  // No compression mode
    auto it = std::lower_bound(m_compressX.begin(), m_compressX.end(), x);
    if (it != m_compressX.end() && *it == x)
        return static_cast<int>(it - m_compressX.begin()) + 1;
    return static_cast<int>(it - m_compressX.begin()) + 1;
}

int FenwickTree6::compressY(int y) const
{
    if (m_compressY.isEmpty()) return y;
    auto it = std::lower_bound(m_compressY.begin(), m_compressY.end(), y);
    if (it != m_compressY.end() && *it == y)
        return static_cast<int>(it - m_compressY.begin()) + 1;
    return static_cast<int>(it - m_compressY.begin()) + 1;
}

/* ---- 2D Update ---- */

void FenwickTree6::update(int x, int y, double value)
{
    QElapsedTimer timer;
    timer.start();

    // Apply compression if in compressed mode
    int cx = m_compressX.isEmpty() ? x + 1 : compressX(x);
    int cy = m_compressY.isEmpty() ? y + 1 : compressY(y);

    for (int i = cx; i <= m_sizeX; i += lsb(i)) {
        for (int j = cy; j <= m_sizeY; j += lsb(j)) {
            m_tree[i][j] += value;
        }
    }

    m_stats.numUpdates++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
}

/* ---- 1D Update ---- */

void FenwickTree6::update1D(int idx, double value)
{
    if (m_tree1D.isEmpty()) {
        m_size1D = qMax(1, idx + 10);
        m_tree1D.fill(0.0, m_size1D + 1);
    }
    int ci = idx + 1;
    for (int i = ci; i <= m_size1D; i += lsb(i))
        m_tree1D[i] += value;
    m_stats.numUpdates++;
    m_stats.totalOps++;
}

/* ---- 2D Prefix sum query ---- */

double FenwickTree6::query(int x, int y) const
{
    double sum = 0.0;
    int cx = m_compressX.isEmpty() ? x + 1 : compressX(x);
    int cy = m_compressY.isEmpty() ? y + 1 : compressY(y);

    cx = qMin(cx, m_sizeX);
    cy = qMin(cy, m_sizeY);

    for (int i = cx; i > 0; i -= lsb(i)) {
        for (int j = cy; j > 0; j -= lsb(j)) {
            sum += m_tree[i][j];
        }
    }
    return sum;
}

/* ---- 2D Range query ---- */

double FenwickTree6::rangeQuery(int x1, int y1, int x2, int y2) const
{
    QElapsedTimer timer;
    timer.start();

    // Inclusion-exclusion: sum(x2,y2) - sum(x1-1,y2) - sum(x2,y1-1) + sum(x1-1,y1-1)
    double result = query(x2, y2) - query(x1 - 1, y2)
                  - query(x2, y1 - 1) + query(x1 - 1, y1 - 1);

    auto self = const_cast<FenwickTree6*>(this);
    self->m_stats.numQueries++;
    self->m_stats.totalOps++;
    self->m_timeSum += timer.elapsed();
    self->m_stats.avgProcessingTimeMs = self->m_timeSum / self->m_stats.totalOps;

    return result;
}

/* ---- 1D Prefix sum ---- */

double FenwickTree6::query1D(int idx) const
{
    if (m_tree1D.isEmpty()) return 0.0;
    double sum = 0.0;
    int ci = qMin(idx + 1, m_size1D);
    for (int i = ci; i > 0; i -= lsb(i))
        sum += m_tree1D[i];
    return sum;
}

/* ---- 1D Range sum ---- */

double FenwickTree6::rangeQuery1D(int l, int r) const
{
    double result = query1D(r) - (l > 0 ? query1D(l - 1) : 0.0);
    auto self = const_cast<FenwickTree6*>(this);
    self->m_stats.numQueries++;
    self->m_stats.totalOps++;
    return result;
}

/* ---- Point query (value at single cell) ---- */

double FenwickTree6::pointQuery(int x, int y) const
{
    // Value at (x,y) = prefix_sum(x,y) - prefix_sum(x-1,y)
    //                - prefix_sum(x,y-1) + prefix_sum(x-1,y-1)
    return rangeQuery(x, y, x, y);
}

/* ---- Clear ---- */

void FenwickTree6::clear()
{
    for (auto& row : m_tree)
        row.fill(0.0);
    for (auto& v : m_tree1D) v = 0.0;
    m_stats.numUpdates = 0;
    m_stats.numQueries = 0;
}

/* ---- Reset ---- */

void FenwickTree6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_tree.clear();
    m_tree1D.clear();
    m_compressX.clear();
    m_compressY.clear();
    m_sizeX = 0;
    m_sizeY = 0;
    m_size1D = 0;
}
