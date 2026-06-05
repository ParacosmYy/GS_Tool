/**
 * @file PersistentSegmentTree.cpp
 * @brief 可持久化(函数式)线段树实现
 */

#include "PersistentSegmentTree.h"
#include <QElapsedTimer>
#include <algorithm>
#include <limits>

PersistentSegmentTree::PersistentSegmentTree(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
    , m_size(0)
{
}

int PersistentSegmentTree::build(const QVector<double>& values)
{
    QElapsedTimer timer;
    timer.start();

    m_size = values.size();
    m_pool.clear();
    m_roots.clear();

    if (m_size == 0) {
        m_roots.append(-1);
        m_timeSum += timer.elapsed();
        return 0;
    }

    int root = buildRec(0, m_size - 1, values);
    m_roots.append(root);

    m_timeSum += timer.elapsed();
    return 0;
}

int PersistentSegmentTree::update(int version, int index, double value)
{
    QElapsedTimer timer;
    timer.start();

    if (version < 0 || version >= m_roots.size() || m_size == 0) {
        m_timeSum += timer.elapsed();
        return m_roots.isEmpty() ? -1 : m_roots.size() - 1;
    }
    if (index < 0 || index >= m_size) {
        m_timeSum += timer.elapsed();
        return version;
    }

    int newRoot = updateRec(m_roots[version], 0, m_size - 1, index, value);
    m_roots.append(newRoot);
    m_stats.totalUpdates++;

    m_timeSum += timer.elapsed();
    int total = m_stats.totalUpdates + m_stats.totalQueries;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit versionCreated(m_roots.size() - 1);
    return m_roots.size() - 1;
}

double PersistentSegmentTree::rangeSum(int version, int left, int right)
{
    QElapsedTimer timer;
    timer.start();

    double result = 0.0;
    if (version >= 0 && version < m_roots.size() && m_roots[version] >= 0) {
        left = qBound(0, left, m_size - 1);
        right = qBound(0, right, m_size - 1);
        if (left <= right)
            result = sumRec(m_roots[version], 0, m_size - 1, left, right);
    }

    m_stats.totalQueries++;
    m_timeSum += timer.elapsed();
    int total = m_stats.totalUpdates + m_stats.totalQueries;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    return result;
}

double PersistentSegmentTree::rangeMin(int version, int left, int right)
{
    QElapsedTimer timer;
    timer.start();

    double result = std::numeric_limits<double>::max();
    if (version >= 0 && version < m_roots.size() && m_roots[version] >= 0) {
        left = qBound(0, left, m_size - 1);
        right = qBound(0, right, m_size - 1);
        if (left <= right)
            result = minRec(m_roots[version], 0, m_size - 1, left, right);
    }

    m_stats.totalQueries++;
    m_timeSum += timer.elapsed();
    int total = m_stats.totalUpdates + m_stats.totalQueries;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    return result;
}

double PersistentSegmentTree::rangeMax(int version, int left, int right)
{
    QElapsedTimer timer;
    timer.start();

    double result = std::numeric_limits<double>::lowest();
    if (version >= 0 && version < m_roots.size() && m_roots[version] >= 0) {
        left = qBound(0, left, m_size - 1);
        right = qBound(0, right, m_size - 1);
        if (left <= right)
            result = maxRec(m_roots[version], 0, m_size - 1, left, right);
    }

    m_stats.totalQueries++;
    m_timeSum += timer.elapsed();
    int total = m_stats.totalUpdates + m_stats.totalQueries;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    return result;
}

int PersistentSegmentTree::versionCount() const
{
    return m_roots.size();
}

double PersistentSegmentTree::pointQuery(int version, int index)
{
    return rangeSum(version, index, index);
}

int PersistentSegmentTree::buildRec(int lo, int hi,
                                    const QVector<double>& values)
{
    int idx = allocNode();

    if (lo == hi) {
        m_pool[idx].sum = values[lo];
        m_pool[idx].minV = values[lo];
        m_pool[idx].maxV = values[lo];
        return idx;
    }

    int mid = lo + (hi - lo) / 2;
    m_pool[idx].left = buildRec(lo, mid, values);
    m_pool[idx].right = buildRec(mid + 1, hi, values);

    const Node& lc = m_pool[m_pool[idx].left];
    const Node& rc = m_pool[m_pool[idx].right];
    m_pool[idx].sum = lc.sum + rc.sum;
    m_pool[idx].minV = std::min(lc.minV, rc.minV);
    m_pool[idx].maxV = std::max(lc.maxV, rc.maxV);

    return idx;
}

int PersistentSegmentTree::updateRec(int nodeIdx, int lo, int hi,
                                     int pos, double val)
{
    int newIdx = allocNode();
    m_pool[newIdx] = m_pool[nodeIdx];

    if (lo == hi) {
        m_pool[newIdx].sum = val;
        m_pool[newIdx].minV = val;
        m_pool[newIdx].maxV = val;
        return newIdx;
    }

    int mid = lo + (hi - lo) / 2;
    if (pos <= mid) {
        m_pool[newIdx].left = updateRec(m_pool[nodeIdx].left, lo, mid, pos, val);
    } else {
        m_pool[newIdx].right = updateRec(m_pool[nodeIdx].right, mid + 1, hi, pos, val);
    }

    const Node& lc = m_pool[m_pool[newIdx].left];
    const Node& rc = m_pool[m_pool[newIdx].right];
    m_pool[newIdx].sum = lc.sum + rc.sum;
    m_pool[newIdx].minV = std::min(lc.minV, rc.minV);
    m_pool[newIdx].maxV = std::max(lc.maxV, rc.maxV);

    return newIdx;
}

double PersistentSegmentTree::sumRec(int nodeIdx, int lo, int hi,
                                     int ql, int qr) const
{
    if (nodeIdx < 0 || ql > hi || qr < lo) return 0.0;
    if (ql <= lo && hi <= qr) return m_pool[nodeIdx].sum;

    int mid = lo + (hi - lo) / 2;
    double leftSum = sumRec(m_pool[nodeIdx].left, lo, mid, ql, qr);
    double rightSum = sumRec(m_pool[nodeIdx].right, mid + 1, hi, ql, qr);
    return leftSum + rightSum;
}

double PersistentSegmentTree::minRec(int nodeIdx, int lo, int hi,
                                     int ql, int qr) const
{
    if (nodeIdx < 0 || ql > hi || qr < lo)
        return std::numeric_limits<double>::max();
    if (ql <= lo && hi <= qr) return m_pool[nodeIdx].minV;

    int mid = lo + (hi - lo) / 2;
    double lMin = minRec(m_pool[nodeIdx].left, lo, mid, ql, qr);
    double rMin = minRec(m_pool[nodeIdx].right, mid + 1, hi, ql, qr);
    return std::min(lMin, rMin);
}

double PersistentSegmentTree::maxRec(int nodeIdx, int lo, int hi,
                                     int ql, int qr) const
{
    if (nodeIdx < 0 || ql > hi || qr < lo)
        return std::numeric_limits<double>::lowest();
    if (ql <= lo && hi <= qr) return m_pool[nodeIdx].maxV;

    int mid = lo + (hi - lo) / 2;
    double lMax = maxRec(m_pool[nodeIdx].left, lo, mid, ql, qr);
    double rMax = maxRec(m_pool[nodeIdx].right, mid + 1, hi, ql, qr);
    return std::max(lMax, rMax);
}

int PersistentSegmentTree::allocNode()
{
    int idx = m_pool.size();
    m_pool.append(Node{});
    return idx;
}

PersistentSegmentTree::Stats PersistentSegmentTree::stats() const
{
    return m_stats;
}

void PersistentSegmentTree::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
