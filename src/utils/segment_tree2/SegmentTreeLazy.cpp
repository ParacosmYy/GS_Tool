/**
 * @file SegmentTreeLazy.cpp
 * @brief 懒标记线段树实现 — 区间更新+区间查询
 */

#include "utils/segment_tree2/SegmentTreeLazy.h"

#include <QElapsedTimer>

#include <algorithm>

/** @brief 构造函数 @param n 数组大小 @param opType 操作类型 @param parent 父对象 */
SegmentTreeLazy::SegmentTreeLazy(int n, OpType opType, QObject* parent)
    : QObject(parent)
    , m_n(std::max(1, n))
    , m_opType(opType)
{
    int sz = 4 * m_n;
    m_tree.assign(sz, 0);
    m_lazyAdd.assign(sz, 0);
    m_hasSet.assign(sz, false);
    m_lazySet.assign(sz, 0);

    /* Min/Max初始值 */
    if (opType == OpType::Min) {
        std::fill(m_tree.begin(), m_tree.end(),
                  std::numeric_limits<qint64>::max());
    } else if (opType == OpType::Max) {
        std::fill(m_tree.begin(), m_tree.end(),
                  std::numeric_limits<qint64>::min());
    }
}

/** @brief 合并值 @param a 值A @param b 值B @return 合并结果 */
qint64 SegmentTreeLazy::merge(qint64 a, qint64 b) const
{
    switch (m_opType) {
    case OpType::Min: return std::min(a, b);
    case OpType::Max: return std::max(a, b);
    case OpType::Sum: return a + b;
    }
    return a + b;
}

/** @brief 上推合并 @param idx 节点索引 */
void SegmentTreeLazy::pushUp(int idx)
{
    m_tree[idx] = merge(m_tree[idx * 2], m_tree[idx * 2 + 1]);
}

/** @brief 下推懒标记 @param idx 节点 @param l 左 @param r 右 */
void SegmentTreeLazy::pushDown(int idx, int l, int r)
{
    int mid = (l + r) / 2;
    int left = idx * 2;
    int right = idx * 2 + 1;
    int leftLen = mid - l + 1;
    int rightLen = r - mid;

    /* 赋值标记优先于加法标记 */
    if (m_hasSet[idx]) {
        qint64 setVal = m_lazySet[idx];

        m_hasSet[left] = true;
        m_lazySet[left] = setVal;
        m_lazyAdd[left] = 0;
        if (m_opType == OpType::Sum) {
            m_tree[left] = setVal * leftLen;
        } else {
            m_tree[left] = setVal;
        }

        m_hasSet[right] = true;
        m_lazySet[right] = setVal;
        m_lazyAdd[right] = 0;
        if (m_opType == OpType::Sum) {
            m_tree[right] = setVal * rightLen;
        } else {
            m_tree[right] = setVal;
        }

        m_hasSet[idx] = false;
    }

    /* 加法标记 */
    if (m_lazyAdd[idx] != 0) {
        qint64 addVal = m_lazyAdd[idx];

        if (m_opType == OpType::Sum) {
            m_tree[left] += addVal * leftLen;
            m_tree[right] += addVal * rightLen;
        } else {
            m_tree[left] += addVal;
            m_tree[right] += addVal;
        }

        m_lazyAdd[left] += addVal;
        m_lazyAdd[right] += addVal;
        m_lazyAdd[idx] = 0;
    }
}

/** @brief 内部构建 @param idx 节点 @param l 左 @param r 右 @param data 数据 */
void SegmentTreeLazy::buildInternal(int idx, int l, int r,
                                     const std::vector<qint64>& data)
{
    m_lazyAdd[idx] = 0;
    m_hasSet[idx] = false;

    if (l == r) {
        m_tree[idx] = data[l];
        return;
    }

    int mid = (l + r) / 2;
    buildInternal(idx * 2, l, mid, data);
    buildInternal(idx * 2 + 1, mid + 1, r, data);
    pushUp(idx);
}

/** @brief 从数组构建 @param data 初始数组 */
void SegmentTreeLazy::build(const QVector<qint64>& data)
{
    if (data.isEmpty()) return;
    m_n = data.size();

    int sz = 4 * m_n;
    m_tree.assign(sz, 0);
    m_lazyAdd.assign(sz, 0);
    m_hasSet.assign(sz, false);
    m_lazySet.assign(sz, 0);

    if (m_opType == OpType::Min) {
        std::fill(m_tree.begin(), m_tree.end(),
                  std::numeric_limits<qint64>::max());
    } else if (m_opType == OpType::Max) {
        std::fill(m_tree.begin(), m_tree.end(),
                  std::numeric_limits<qint64>::min());
    }

    std::vector<qint64> v(data.begin(), data.end());
    buildInternal(1, 0, m_n - 1, v);
}

/** @brief 内部区间加 */
void SegmentTreeLazy::updateAdd(int idx, int l, int r, int ql, int qr, qint64 val)
{
    ++m_stats.totalNodesVisited;
    if (ql <= l && r <= qr) {
        if (m_opType == OpType::Sum) {
            m_tree[idx] += val * (r - l + 1);
        } else {
            m_tree[idx] += val;
        }
        m_lazyAdd[idx] += val;
        return;
    }

    pushDown(idx, l, r);
    int mid = (l + r) / 2;
    if (ql <= mid) updateAdd(idx * 2, l, mid, ql, qr, val);
    if (qr > mid) updateAdd(idx * 2 + 1, mid + 1, r, ql, qr, val);
    pushUp(idx);
}

/** @brief 内部区间赋值 */
void SegmentTreeLazy::updateSet(int idx, int l, int r, int ql, int qr, qint64 val)
{
    ++m_stats.totalNodesVisited;
    if (ql <= l && r <= qr) {
        m_hasSet[idx] = true;
        m_lazySet[idx] = val;
        m_lazyAdd[idx] = 0;
        if (m_opType == OpType::Sum) {
            m_tree[idx] = val * (r - l + 1);
        } else {
            m_tree[idx] = val;
        }
        return;
    }

    pushDown(idx, l, r);
    int mid = (l + r) / 2;
    if (ql <= mid) updateSet(idx * 2, l, mid, ql, qr, val);
    if (qr > mid) updateSet(idx * 2 + 1, mid + 1, r, ql, qr, val);
    pushUp(idx);
}

/** @brief 内部区间查询 */
qint64 SegmentTreeLazy::query(int idx, int l, int r, int ql, int qr)
{
    ++m_stats.totalNodesVisited;
    if (ql <= l && r <= qr) return m_tree[idx];

    pushDown(idx, l, r);
    int mid = (l + r) / 2;

    bool leftVisited = false, rightVisited = false;
    qint64 leftVal = 0, rightVal = 0;

    if (m_opType == OpType::Min) {
        leftVal = std::numeric_limits<qint64>::max();
        rightVal = std::numeric_limits<qint64>::max();
    } else if (m_opType == OpType::Max) {
        leftVal = std::numeric_limits<qint64>::min();
    }

    if (ql <= mid) {
        leftVal = query(idx * 2, l, mid, ql, qr);
        leftVisited = true;
    }
    if (qr > mid) {
        rightVal = query(idx * 2 + 1, mid + 1, r, ql, qr);
        rightVisited = true;
    }

    if (!leftVisited) return rightVal;
    if (!rightVisited) return leftVal;
    return merge(leftVal, rightVal);
}

/** @brief 区间加 @param l 左 @param r 右 @param val 增量 */
void SegmentTreeLazy::rangeAdd(int l, int r, qint64 val)
{
    if (l < 0 || r >= m_n || l > r) return;

    QElapsedTimer timer;
    timer.start();

    updateAdd(1, 0, m_n - 1, l, r, val);

    ++m_stats.totalUpdates;
    m_timeSumMs += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSumMs
        / static_cast<double>(m_stats.totalUpdates + m_stats.totalQueries);

    emit rangeUpdated(l, r, val);
}

/** @brief 区间赋值 @param l 左 @param r 右 @param val 值 */
void SegmentTreeLazy::rangeSet(int l, int r, qint64 val)
{
    if (l < 0 || r >= m_n || l > r) return;

    QElapsedTimer timer;
    timer.start();

    updateSet(1, 0, m_n - 1, l, r, val);

    ++m_stats.totalUpdates;
    m_timeSumMs += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSumMs
        / static_cast<double>(m_stats.totalUpdates + m_stats.totalQueries);

    emit rangeUpdated(l, r, val);
}

/** @brief 区间查询 @param l 左 @param r 右 @return 结果 */
qint64 SegmentTreeLazy::rangeQuery(int l, int r)
{
    if (l < 0 || r >= m_n || l > r) {
        return (m_opType == OpType::Sum) ? 0 : 0;
    }

    QElapsedTimer timer;
    timer.start();

    qint64 result = query(1, 0, m_n - 1, l, r);

    ++m_stats.totalQueries;
    m_timeSumMs += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSumMs
        / static_cast<double>(m_stats.totalUpdates + m_stats.totalQueries);

    emit rangeQueried(l, r, result);
    return result;
}

/** @brief 单点查询 @param idx 索引 @return 值 */
qint64 SegmentTreeLazy::pointQuery(int idx)
{
    if (idx < 0 || idx >= m_n) return 0;
    return rangeQuery(idx, idx);
}

/** @brief 单点更新 @param idx 索引 @param val 增量 */
void SegmentTreeLazy::pointAdd(int idx, qint64 val)
{
    rangeAdd(idx, idx, val);
}

/** @brief 重置统计 */
void SegmentTreeLazy::resetStatistics()
{
    m_stats = Stats{};
    m_timeSumMs = 0.0;
}
