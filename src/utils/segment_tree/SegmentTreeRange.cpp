/**
 * @file SegmentTreeRange.cpp
 * @brief 线段树实现 — 区间求和查询与单点更新
 */

#include "utils/segment_tree/SegmentTreeRange.h"

#include <QElapsedTimer>
#include <algorithm>

// ============================================================================
// 构造
// ============================================================================

SegmentTreeRange::SegmentTreeRange(QObject* parent)
    : QObject(parent)
    , m_size(0)
    , m_timeSum(0.0)
{
}

// ============================================================================
// 公开方法
// ============================================================================

void SegmentTreeRange::build(QVector<double> data)
{
    m_data = std::move(data);
    m_size = m_data.size();

    /* 线段树需要 4*n 空间 */
    m_tree.resize(static_cast<int>(m_size) * 4, 0.0);

    if (m_size > 0) {
        buildImpl(1, 0, m_size - 1);
    }
}

double SegmentTreeRange::rangeSum(int left, int right)
{
    QElapsedTimer timer;
    timer.start();

    double result = 0.0;
    if (m_size > 0 && left >= 0 && right < m_size && left <= right) {
        result = queryImpl(1, 0, m_size - 1, left, right);
    }

    ++m_stats.totalQueries;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs =
        (m_stats.totalQueries + m_stats.totalUpdates > 0)
        ? m_timeSum / (m_stats.totalQueries + m_stats.totalUpdates) : 0.0;

    emit queryCompleted(result);
    return result;
}

void SegmentTreeRange::pointUpdate(int index, double value)
{
    QElapsedTimer timer;
    timer.start();

    if (index >= 0 && index < m_size) {
        m_data[index] = value;
        updateImpl(1, 0, m_size - 1, index, value);
    }

    ++m_stats.totalUpdates;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs =
        (m_stats.totalQueries + m_stats.totalUpdates > 0)
        ? m_timeSum / (m_stats.totalQueries + m_stats.totalUpdates) : 0.0;
}

void SegmentTreeRange::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

// ============================================================================
// 内部实现
// ============================================================================

void SegmentTreeRange::buildImpl(int node, int start, int end)
{
    if (start == end) {
        /* 叶节点: 存储原始数据 */
        m_tree[node] = m_data[start];
        return;
    }

    int mid = start + (end - start) / 2;
    buildImpl(node * 2, start, mid);
    buildImpl(node * 2 + 1, mid + 1, end);

    /* 内部节点: 存储子节点之和 */
    m_tree[node] = m_tree[node * 2] + m_tree[node * 2 + 1];
}

double SegmentTreeRange::queryImpl(int node, int start, int end, int l, int r)
{
    /* 完全在查询范围外 */
    if (r < start || l > end) {
        return 0.0;
    }

    /* 完全在查询范围内 */
    if (l <= start && end <= r) {
        return m_tree[node];
    }

    /* 部分重叠: 递归查询子节点 */
    int mid = start + (end - start) / 2;
    double leftSum = queryImpl(node * 2, start, mid, l, r);
    double rightSum = queryImpl(node * 2 + 1, mid + 1, end, l, r);
    return leftSum + rightSum;
}

void SegmentTreeRange::updateImpl(int node, int start, int end, int idx, double val)
{
    if (start == end) {
        m_tree[node] = val;
        return;
    }

    int mid = start + (end - start) / 2;
    if (idx <= mid) {
        updateImpl(node * 2, start, mid, idx, val);
    } else {
        updateImpl(node * 2 + 1, mid + 1, end, idx, val);
    }

    /* 更新内部节点 */
    m_tree[node] = m_tree[node * 2] + m_tree[node * 2 + 1];
}
