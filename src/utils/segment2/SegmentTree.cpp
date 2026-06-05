/**
 * @file SegmentTree.cpp
 * @brief 线段树实现
 */

#include "utils/segment2/SegmentTree.h"

#include <QElapsedTimer>
#include <cmath>
#include <limits>

SegmentTree::SegmentTree(QueryType type, QObject* parent)
    : QObject(parent), m_type(type), m_size(0), m_timeSum(0.0) {}

void SegmentTree::build(const QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();

    m_data = data;
    m_size = data.size();

    /* 分配4N空间 */
    m_tree.resize(4 * m_size, 0.0);
    if (m_size > 0) buildImpl(1, 0, m_size - 1);

    m_stats.treeSize = m_tree.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalQueries + m_stats.totalUpdates, 1ULL);
}

double SegmentTree::query(int left, int right)
{
    QElapsedTimer timer;
    timer.start();

    if (m_size == 0 || left < 0 || right >= m_size || left > right) {
        return neutralElement();
    }

    double result = queryImpl(1, 0, m_size - 1, left, right);

    m_stats.totalQueries++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalQueries + m_stats.totalUpdates, 1ULL);

    emit queried(left, right, result);
    return result;
}

void SegmentTree::update(int index, double value)
{
    QElapsedTimer timer;
    timer.start();

    if (index < 0 || index >= m_size) return;

    m_data[index] = value;
    updateImpl(1, 0, m_size - 1, index, value);

    m_stats.totalUpdates++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalQueries + m_stats.totalUpdates, 1ULL);

    emit updated(index, value);
}

void SegmentTree::updateDelta(int index, double delta)
{
    if (index < 0 || index >= m_size) return;
    update(index, m_data[index] + delta);
}

void SegmentTree::buildImpl(int node, int start, int end)
{
    if (start == end) {
        m_tree[node] = m_data[start];
        return;
    }

    int mid = (start + end) / 2;
    buildImpl(2 * node, start, mid);
    buildImpl(2 * node + 1, mid + 1, end);
    m_tree[node] = combine(m_tree[2 * node], m_tree[2 * node + 1]);
}

double SegmentTree::queryImpl(int node, int start, int end, int l, int r)
{
    /* 完全不相交 */
    if (r < start || l > end) return neutralElement();

    /* 完全包含 */
    if (l <= start && end <= r) return m_tree[node];

    /* 部分重叠 */
    int mid = (start + end) / 2;
    double leftResult  = queryImpl(2 * node, start, mid, l, r);
    double rightResult = queryImpl(2 * node + 1, mid + 1, end, l, r);
    return combine(leftResult, rightResult);
}

void SegmentTree::updateImpl(int node, int start, int end, int idx, double val)
{
    if (start == end) {
        m_tree[node] = val;
        return;
    }

    int mid = (start + end) / 2;
    if (idx <= mid) {
        updateImpl(2 * node, start, mid, idx, val);
    } else {
        updateImpl(2 * node + 1, mid + 1, end, idx, val);
    }
    m_tree[node] = combine(m_tree[2 * node], m_tree[2 * node + 1]);
}

double SegmentTree::neutralElement() const
{
    switch (m_type) {
    case QueryType::Sum: return 0.0;
    case QueryType::Min: return std::numeric_limits<double>::max();
    case QueryType::Max: return std::numeric_limits<double>::lowest();
    }
    return 0.0;
}

double SegmentTree::combine(double a, double b) const
{
    switch (m_type) {
    case QueryType::Sum: return a + b;
    case QueryType::Min: return std::min(a, b);
    case QueryType::Max: return std::max(a, b);
    }
    return a + b;
}

void SegmentTree::resetStatistics()
{
    m_stats = Stats{};
    m_stats.treeSize = m_tree.size();
    m_timeSum = 0.0;
}
