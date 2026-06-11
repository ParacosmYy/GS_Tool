/**
 * @file FenwickTree12.cpp
 * @brief FenwickTree12 实现
 *
 * 实现树状数组：二维范围查询与坐标压缩实现多维前缀和操作。
 */

#include "utils/tree296/FenwickTree12.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

FenwickTree12::FenwickTree12(QObject *parent)
    : QObject(parent) {}

FenwickTree12::~FenwickTree12() = default;

/* ---- Initialize 2D Fenwick tree ---- */

void FenwickTree12::init(int rows, int cols)
{
    m_rows = qBound(1, rows, 10000);
    m_cols = qBound(1, cols, 10000);
    m_numPoints = 0;

    m_tree.resize(m_rows + 1);
    m_original.resize(m_rows + 1);
    for (int i = 0; i <= m_rows; ++i) {
        m_tree[i].resize(m_cols + 1, 0.0);
        m_original[i].resize(m_cols + 1, 0.0);
    }
}

/* ---- Add value at (r, c) ---- */

void FenwickTree12::add(int r, int c, double value)
{
    QElapsedTimer timer;
    timer.start();

    // 1-indexed internally
    for (int i = r + 1; i <= m_rows; i += lsb(i)) {
        for (int j = c + 1; j <= m_cols; j += lsb(j)) {
            m_tree[i][j] += value;
        }
    }
    m_numPoints++;

    double elapsed = timer.elapsed();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    m_stats.treeRows = m_rows;
    m_stats.treeCols = m_cols;
    m_stats.numPoints = m_numPoints;

    emit operationDone(QStringLiteral("add"), r, c, elapsed);
}

/* ---- Prefix sum from (1,1) to (r, c) ---- */

double FenwickTree12::prefixSum(int r, int c) const
{
    double sum = 0.0;
    // Input is 0-indexed, convert to 1-indexed
    for (int i = r + 1; i > 0; i -= lsb(i)) {
        for (int j = c + 1; j > 0; j -= lsb(j)) {
            sum += m_tree[i][j];
        }
    }
    return sum;
}

/* ---- Range sum query from (r1,c1) to (r2,c2) ---- */

double FenwickTree12::rangeSum(int r1, int c1, int r2, int c2) const
{
    QElapsedTimer timer;
    timer.start();

    // Inclusion-exclusion for 2D range sum
    double sum = prefixSum(r2, c2);
    if (r1 > 0) sum -= prefixSum(r1 - 1, c2);
    if (c1 > 0) sum -= prefixSum(r2, c1 - 1);
    if (r1 > 0 && c1 > 0) sum += prefixSum(r1 - 1, c1 - 1);

    double elapsed = timer.elapsed();
    m_stats.totalOps++;

    emit operationDone(QStringLiteral("rangeSum"), r1, c1, elapsed);
    return sum;
}

/* ---- Compress coordinates ---- */

void FenwickTree12::compressCoordinates(QVector<int>& xs, QVector<int>& ys)
{
    // Collect unique sorted values
    QVector<int> uniqueX = xs;
    std::sort(uniqueX.begin(), uniqueX.end());
    uniqueX.erase(std::unique(uniqueX.begin(), uniqueX.end()), uniqueX.end());
    m_compressedX = uniqueX;

    QVector<int> uniqueY = ys;
    std::sort(uniqueY.begin(), uniqueY.end());
    uniqueY.erase(std::unique(uniqueY.begin(), uniqueY.end()), uniqueY.end());
    m_compressedY = uniqueY;

    // Map original to compressed (1-indexed)
    for (int i = 0; i < xs.size(); ++i) {
        auto itX = std::lower_bound(m_compressedX.begin(), m_compressedX.end(), xs[i]);
        xs[i] = static_cast<int>(itX - m_compressedX.begin());

        auto itY = std::lower_bound(m_compressedY.begin(), m_compressedY.end(), ys[i]);
        ys[i] = static_cast<int>(itY - m_compressedY.begin());
    }

    // Reinitialize tree with compressed dimensions
    init(m_compressedX.size(), m_compressedY.size());
}

/* ---- Point query at (r, c) ---- */

double FenwickTree12::pointQuery(int r, int c) const
{
    double val = prefixSum(r, c);
    if (r > 0) val -= prefixSum(r - 1, c);
    if (c > 0) val -= prefixSum(r, c - 1);
    if (r > 0 && c > 0) val += prefixSum(r - 1, c - 1);
    return val;
}

/* ---- Point update: set value at (r, c) ---- */

void FenwickTree12::pointUpdate(int r, int c, double value)
{
    QElapsedTimer timer;
    timer.start();

    // Get current value and compute delta
    double current = pointQuery(r, c);
    double delta = value - current;

    // Apply delta through add operation
    for (int i = r + 1; i <= m_rows; i += lsb(i)) {
        for (int j = c + 1; j <= m_cols; j += lsb(j)) {
            m_tree[i][j] += delta;
        }
    }

    // Store original value
    if (r + 1 < m_original.size() && c + 1 < m_original[r + 1].size())
        m_original[r + 1][c + 1] = value;

    double elapsed = timer.elapsed();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit operationDone(QStringLiteral("pointUpdate"), r, c, elapsed);
}

/* ---- Reset statistics ---- */

void FenwickTree12::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_numPoints = 0;
}
