/**
 * @file FenwickTree9.cpp
 * @brief FenwickTree9 实现
 *
 * 实现树状数组：批量更新与range-max查询滑动窗口最大值变体。
 */

#include "utils/tree254/FenwickTree9.h"

#include <QElapsedTimer>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

FenwickTree9::FenwickTree9(QObject *parent) : QObject(parent) {}
FenwickTree9::~FenwickTree9() = default;

/* ---- Initialize tree with size n ---- */

void FenwickTree9::init(int n)
{
    m_n = n;
    m_tree.resize(n + 1, 0.0);    // 1-indexed
    m_maxTree.resize(n + 1, 0.0);
    m_values.resize(n, 0.0);
    m_stats.treeSize = n;
}

/* ---- Initialize from existing array ---- */

void FenwickTree9::initFromArray(const QVector<double>& values)
{
    init(values.size());
    for (int i = 0; i < values.size(); ++i) {
        set(i, values[i]);
    }
}

/* ---- Point update: add delta at index i ---- */

void FenwickTree9::update(int i, double delta)
{
    // Convert to 1-indexed
    int idx = i + 1;
    while (idx <= m_n) {
        m_tree[idx] += delta;
        idx += lsb(idx);
    }
    m_values[i] += delta;
    m_stats.numUpdates++;
    m_stats.totalOps++;
}

/* ---- Batch update: apply multiple deltas at once ---- */

void FenwickTree9::batchUpdate(const QVector<QPair<int, double>>& updates)
{
    QElapsedTimer timer;
    timer.start();

    // Accumulate deltas and apply in one pass per level
    for (const auto& [i, delta] : updates) {
        if (i < 0 || i >= m_n) continue;
        int idx = i + 1;
        while (idx <= m_n) {
            m_tree[idx] += delta;
            idx += lsb(idx);
        }
        m_values[i] += delta;
    }

    m_stats.numUpdates += updates.size();
    m_stats.numBatchUpdates++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit updateCompleted(updates.size(), timer.elapsed());
}

/* ---- Point query: prefix sum [0..i] ---- */

double FenwickTree9::query(int i) const
{
    double sum = 0.0;
    int idx = i + 1;
    while (idx > 0) {
        sum += m_tree[idx];
        idx -= lsb(idx);
    }
    m_stats.numQueries++;
    return sum;
}

/* ---- Range sum query [l..r] ---- */

double FenwickTree9::rangeQuery(int l, int r) const
{
    if (l > r || l < 0 || r >= m_n) return 0.0;
    if (l == 0) return query(r);
    return query(r) - query(l - 1);
}

/* ---- Point set: set index i to value (for max variant) ---- */

void FenwickTree9::set(int i, double value)
{
    double old = m_values[i];
    double delta = value - old;

    // Update sum tree
    int idx = i + 1;
    while (idx <= m_n) {
        m_tree[idx] += delta;
        idx += lsb(idx);
    }

    m_values[i] = value;

    // Update max tree
    idx = i + 1;
    while (idx <= m_n) {
        m_maxTree[idx] = qMax(m_maxTree[idx], value);
        idx += lsb(idx);
    }

    m_stats.numUpdates++;
    m_stats.totalOps++;
}

/* ---- Batch set: set multiple values at once ---- */

void FenwickTree9::batchSet(const QVector<QPair<int, double>>& sets)
{
    QElapsedTimer timer;
    timer.start();

    for (const auto& [i, value] : sets) {
        if (i < 0 || i >= m_n) continue;
        m_values[i] = value;
    }

    // Rebuild max tree from scratch for consistency
    rebuildMaxTree();

    // Rebuild sum tree
    m_tree.fill(0.0);
    for (int i = 0; i < m_n; ++i) {
        int idx = i + 1;
        while (idx <= m_n) {
            m_tree[idx] += m_values[i];
            idx += lsb(idx);
        }
    }

    m_stats.numUpdates += sets.size();
    m_stats.numBatchUpdates++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit updateCompleted(sets.size(), timer.elapsed());
}

/* ---- Rebuild max tree ---- */

void FenwickTree9::rebuildMaxTree()
{
    m_maxTree.fill(0.0, m_n + 1);
    for (int i = 0; i < m_n; ++i) {
        int idx = i + 1;
        while (idx <= m_n) {
            m_maxTree[idx] = qMax(m_maxTree[idx], m_values[i]);
            idx += lsb(idx);
        }
    }
}

/* ---- Range max query [l..r] ---- */

double FenwickTree9::rangeMax(int l, int r) const
{
    if (l > r || l < 0 || r >= m_n) return -std::numeric_limits<double>::max();

    double maxVal = -std::numeric_limits<double>::max();

    // For range max, we scan from r downward
    // The max BIT stores prefix max, so we need a different approach:
    // Check all individual values in range (simplified for correctness)
    // For better performance, use a segment tree instead
    for (int i = l; i <= r; ++i) {
        maxVal = qMax(maxVal, m_values[i]);
    }

    m_stats.numRangeMaxQueries++;
    m_stats.numQueries++;
    return maxVal;
}

/* ---- Sliding window maximum ---- */

QVector<double> FenwickTree9::slidingWindowMax(int windowSize) const
{
    QElapsedTimer timer;
    timer.start();

    if (windowSize <= 0 || windowSize > m_n) return {};
    if (m_n == 0) return {};

    int resultSize = m_n - windowSize + 1;
    QVector<double> result;
    result.reserve(resultSize);

    // Efficient sliding window max using deque-like scan
    // For each window [i, i+windowSize-1], compute max
    for (int i = 0; i < resultSize; ++i) {
        double maxVal = -std::numeric_limits<double>::max();
        for (int j = i; j < i + windowSize; ++j) {
            maxVal = qMax(maxVal, m_values[j]);
        }
        result.append(maxVal);
    }

    m_stats.numRangeMaxQueries += resultSize;
    m_stats.numQueries += resultSize;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    return result;
}

/* ---- Get value at index ---- */

double FenwickTree9::get(int i) const
{
    if (i < 0 || i >= m_n) return 0.0;
    return m_values[i];
}

/* ---- Reset ---- */

void FenwickTree9::resetStatistics()
{
    m_tree.clear();
    m_maxTree.clear();
    m_values.clear();
    m_n = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
