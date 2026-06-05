/**
 * @file LazySegmentTree.cpp
 * @brief 懒传播线段树实现 — 区间更新/区间查询
 */

#include "utils/segment4/LazySegmentTree.h"

#include <QElapsedTimer>
#include <algorithm>
#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
LazySegmentTree::LazySegmentTree(QObject* parent)
    : QObject(parent)
{
}

/** @brief 用初始数据构建线段树 @param data 初始数据 */
void LazySegmentTree::build(const QVector<double>& data)
{
    if (data.isEmpty()) return;

    QElapsedTimer timer;
    timer.start();

    m_size = data.size();
    /* 线段树需要4倍空间 */
    m_tree.resize(4 * m_size);

    buildHelper(1, 0, m_size - 1, data);

    qint64 elapsed = timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);
    m_stats.avgProcessingTimeMs = (m_stats.totalUpdates + m_stats.totalQueries > 0)
        ? m_timeSum / (m_stats.totalUpdates + m_stats.totalQueries) : 0.0;
}

/** @brief 区间更新 @param left 左端 @param right 右端 @param value 更新值 @param op 操作 */
void LazySegmentTree::rangeUpdate(int left, int right, double value, UpdateOp op)
{
    if (!isBuilt() || left > right || left < 0 || right >= m_size) return;

    QElapsedTimer timer;
    timer.start();

    updateHelper(1, 0, m_size - 1, left, right, value, op);

    ++m_stats.totalUpdates;

    qint64 elapsed = timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);
    m_stats.avgProcessingTimeMs = m_timeSum
        / (m_stats.totalUpdates + m_stats.totalQueries);

    emit rangeUpdated(left, right, value);
}

/** @brief 区间查询 @param left 左端 @param right 右端 @param type 查询类型 @return 结果 */
LazySegmentTree::QueryResult LazySegmentTree::rangeQuery(
    int left, int right, QueryType type)
{
    if (!isBuilt() || left > right || left < 0 || right >= m_size) {
        return QueryResult{};
    }

    QElapsedTimer timer;
    timer.start();

    QueryResult result;
    result.min = std::numeric_limits<double>::max();
    result.max = -std::numeric_limits<double>::max();
    result.sum = 0.0;
    result.count = right - left + 1;

    queryHelper(1, 0, m_size - 1, left, right, result);

    ++m_stats.totalQueries;

    qint64 elapsed = timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);
    m_stats.avgProcessingTimeMs = m_timeSum
        / (m_stats.totalUpdates + m_stats.totalQueries);

    emit rangeQueried(left, right, result);
    return result;
}

/** @brief 单点更新 @param index 位置 @param value 新值 @param op 操作 */
void LazySegmentTree::pointUpdate(int index, double value, UpdateOp op)
{
    if (!isBuilt() || index < 0 || index >= m_size) return;
    rangeUpdate(index, index, value, op);
}

/** @brief 单点查询 @param index 位置 @return 值 */
double LazySegmentTree::pointQuery(int index)
{
    if (!isBuilt() || index < 0 || index >= m_size) return 0.0;
    QueryResult r = rangeQuery(index, index);
    return r.sum;
}

/** @brief 获取原始数据快照 @return 数据 */
QVector<double> LazySegmentTree::data()
{
    if (!isBuilt()) return {};

    QVector<double> result(m_size);
    for (int i = 0; i < m_size; ++i) {
        QueryResult r = rangeQuery(i, i);
        result[i] = r.sum;
    }
    return result;
}

/** @brief 重置统计 */
void LazySegmentTree::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 向上更新 @param idx 节点索引 */
void LazySegmentTree::pushUp(int idx)
{
    int left = idx * 2;
    int right = idx * 2 + 1;

    m_tree[idx].sum = m_tree[left].sum + m_tree[right].sum;
    m_tree[idx].minVal = qMin(m_tree[left].minVal, m_tree[right].minVal);
    m_tree[idx].maxVal = qMax(m_tree[left].maxVal, m_tree[right].maxVal);
}

/** @brief 向下传播懒标记 @param idx 节点索引 @param l 左界 @param r 右界 */
void LazySegmentTree::pushDown(int idx, int l, int r) const
{
    int mid = (l + r) / 2;
    int left = idx * 2;
    int right = idx * 2 + 1;
    int leftLen = mid - l + 1;
    int rightLen = r - mid;

    /* 先处理乘法标记 */
    if (m_tree[idx].hasLazyMul) {
        double mul = m_tree[idx].lazyMul;

        /* 应用到左子 */
        m_tree[left].sum *= mul;
        m_tree[left].minVal *= mul;
        m_tree[left].maxVal *= mul;
        m_tree[left].lazyMul *= mul;
        m_tree[left].hasLazyMul = true;
        if (m_tree[left].hasLazyAssign) {
            m_tree[left].lazyAssign *= mul;
        }
        m_tree[left].lazyAdd *= mul;

        /* 应用到右子 */
        m_tree[right].sum *= mul;
        m_tree[right].minVal *= mul;
        m_tree[right].maxVal *= mul;
        m_tree[right].lazyMul *= mul;
        m_tree[right].hasLazyMul = true;
        if (m_tree[right].hasLazyAssign) {
            m_tree[right].lazyAssign *= mul;
        }
        m_tree[right].lazyAdd *= mul;

        m_tree[idx].lazyMul = 1.0;
        m_tree[idx].hasLazyMul = false;
    }

    /* 再处理赋值标记(覆盖加法) */
    if (m_tree[idx].hasLazyAssign) {
        double val = m_tree[idx].lazyAssign;

        m_tree[left].sum = val * leftLen;
        m_tree[left].minVal = val;
        m_tree[left].maxVal = val;
        m_tree[left].lazyAssign = val;
        m_tree[left].hasLazyAssign = true;
        m_tree[left].lazyAdd = 0.0;

        m_tree[right].sum = val * rightLen;
        m_tree[right].minVal = val;
        m_tree[right].maxVal = val;
        m_tree[right].lazyAssign = val;
        m_tree[right].hasLazyAssign = true;
        m_tree[right].lazyAdd = 0.0;

        m_tree[idx].hasLazyAssign = false;
        m_tree[idx].lazyAssign = 0.0;
    }

    /* 最后处理加法标记 */
    if (m_tree[idx].lazyAdd != 0.0) {
        double add = m_tree[idx].lazyAdd;

        m_tree[left].sum += add * leftLen;
        m_tree[left].minVal += add;
        m_tree[left].maxVal += add;
        m_tree[left].lazyAdd += add;

        m_tree[right].sum += add * rightLen;
        m_tree[right].minVal += add;
        m_tree[right].maxVal += add;
        m_tree[right].lazyAdd += add;

        m_tree[idx].lazyAdd = 0.0;
    }

    ++m_stats.totalPropagations;
}

/** @brief 递归构建 @param idx 节点 @param l 左界 @param r 右界 @param data 数据 */
void LazySegmentTree::buildHelper(int idx, int l, int r,
                                   const QVector<double>& data)
{
    m_tree[idx].lazyAdd = 0.0;
    m_tree[idx].lazyAssign = 0.0;
    m_tree[idx].hasLazyAssign = false;
    m_tree[idx].lazyMul = 1.0;
    m_tree[idx].hasLazyMul = false;

    if (l == r) {
        m_tree[idx].sum = data[l];
        m_tree[idx].minVal = data[l];
        m_tree[idx].maxVal = data[l];
        return;
    }

    int mid = (l + r) / 2;
    buildHelper(idx * 2, l, mid, data);
    buildHelper(idx * 2 + 1, mid + 1, r, data);
    pushUp(idx);
}

/** @brief 递归更新 @param idx 节点 @param l 左界 @param r 右界 @param ql 查询左 @param qr 查询右 @param val 值 @param op 操作 */
void LazySegmentTree::updateHelper(int idx, int l, int r, int ql, int qr,
                                    double val, UpdateOp op)
{
    m_stats.totalNodesVisited++;

    if (ql <= l && r <= qr) {
        int len = r - l + 1;
        switch (op) {
        case UpdateOp::Add:
            m_tree[idx].sum += val * len;
            m_tree[idx].minVal += val;
            m_tree[idx].maxVal += val;
            m_tree[idx].lazyAdd += val;
            break;
        case UpdateOp::Assign:
            m_tree[idx].sum = val * len;
            m_tree[idx].minVal = val;
            m_tree[idx].maxVal = val;
            m_tree[idx].lazyAssign = val;
            m_tree[idx].hasLazyAssign = true;
            m_tree[idx].lazyAdd = 0.0;
            break;
        case UpdateOp::Multiply:
            m_tree[idx].sum *= val;
            m_tree[idx].minVal *= val;
            m_tree[idx].maxVal *= val;
            m_tree[idx].lazyMul *= val;
            m_tree[idx].hasLazyMul = true;
            m_tree[idx].lazyAdd *= val;
            break;
        }
        return;
    }

    pushDown(idx, l, r);
    int mid = (l + r) / 2;

    if (ql <= mid) {
        updateHelper(idx * 2, l, mid, ql, qr, val, op);
    }
    if (qr > mid) {
        updateHelper(idx * 2 + 1, mid + 1, r, ql, qr, val, op);
    }

    pushUp(idx);
}

/** @brief 递归查询 @param idx 节点 @param l 左界 @param r 右界 @param ql 查询左 @param qr 查询右 @param result 结果 */
void LazySegmentTree::queryHelper(int idx, int l, int r, int ql, int qr,
                                   QueryResult& result) const
{
    m_stats.totalNodesVisited++;

    if (ql <= l && r <= qr) {
        result.sum += m_tree[idx].sum;
        result.min = qMin(result.min, m_tree[idx].minVal);
        result.max = qMax(result.max, m_tree[idx].maxVal);
        return;
    }

    pushDown(idx, l, r);
    int mid = (l + r) / 2;

    if (ql <= mid) {
        queryHelper(idx * 2, l, mid, ql, qr, result);
    }
    if (qr > mid) {
        queryHelper(idx * 2 + 1, mid + 1, r, ql, qr, result);
    }
}
