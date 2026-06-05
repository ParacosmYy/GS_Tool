/**
 * @file FenwickTree.cpp
 * @brief 树状数组实现
 */

#include "utils/fenwick/FenwickTree.h"

#include <QElapsedTimer>

FenwickTree::FenwickTree(QObject* parent)
    : QObject(parent), m_size(0), m_timeSum(0.0) {}

void FenwickTree::initialize(int n)
{
    m_size = n;
    m_bit.assign(n + 1, 0.0);
    m_original.assign(n, 0.0);
    m_stats.treeSize = n;
}

void FenwickTree::build(const QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();

    m_size = data.size();
    m_bit.assign(m_size + 1, 0.0);
    m_original = data;

    /* O(N)建树 */
    for (int i = 0; i < m_size; ++i) {
        int bitIdx = i + 1;
        m_bit[bitIdx] += data[i];
        int parent = bitIdx + (bitIdx & (-bitIdx));
        if (parent <= m_size) {
            m_bit[parent] += m_bit[bitIdx];
        }
    }

    m_stats.treeSize = m_size;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalPrefixQueries + m_stats.totalUpdates, 1ULL);
}

void FenwickTree::update(int index, double delta)
{
    QElapsedTimer timer;
    timer.start();

    if (index < 0 || index >= m_size) return;

    m_original[index] += delta;
    int bitIdx = index + 1;
    while (bitIdx <= m_size) {
        m_bit[bitIdx] += delta;
        bitIdx += bitIdx & (-bitIdx);
    }

    m_stats.totalUpdates++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalPrefixQueries + m_stats.totalRangeQueries +
             m_stats.totalUpdates, 1ULL);

    emit updated(index, delta);
}

void FenwickTree::set(int index, double value)
{
    if (index < 0 || index >= m_size) return;
    double delta = value - m_original[index];
    update(index, delta);
}

double FenwickTree::prefixSum(int index) const
{
    if (index < 0 || index >= m_size) return 0.0;

    double sum = 0.0;
    int bitIdx = index + 1;
    while (bitIdx > 0) {
        sum += m_bit[bitIdx];
        bitIdx -= bitIdx & (-bitIdx);
    }

    m_stats.totalPrefixQueries++;
    return sum;
}

double FenwickTree::rangeSum(int left, int right) const
{
    QElapsedTimer timer;
    timer.start();

    if (left < 0) left = 0;
    if (right >= m_size) right = m_size - 1;
    if (left > right) return 0.0;

    double result = prefixSum(right);
    if (left > 0) result -= prefixSum(left - 1);

    m_stats.totalRangeQueries++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalPrefixQueries + m_stats.totalRangeQueries +
             m_stats.totalUpdates, 1ULL);

    const_cast<FenwickTree*>(this)->emit rangeQueried(left, right, result);
    return result;
}

double FenwickTree::valueAt(int index) const
{
    if (index < 0 || index >= m_size) return 0.0;
    return m_original[index];
}

void FenwickTree::resetStatistics()
{
    m_stats = Stats{};
    m_stats.treeSize = m_size;
    m_timeSum = 0.0;
}
