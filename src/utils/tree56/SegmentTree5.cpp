/**
 * @file SegmentTree5.cpp
 * @brief 线段树实现，支持区间求和/最小/最大查询和单点更新
 *
 * 线段树(Segment Tree)是一种平衡二叉树数据结构，
 * 用于高效处理区间查询和点更新操作。
 *
 * 本实现维护三棵线段树：
 * - sumTree: 区间求和
 * - minTree: 区间最小值
 * - maxTree: 区间最大值
 *
 * 时间复杂度：构建O(n)，查询O(log n)，更新O(log n)
 * 空间复杂度：O(n)（4n大小的数组）
 *
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/tree56/SegmentTree5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化空线段树
 * @param parent 父QObject对象指针
 */
SegmentTree5::SegmentTree5(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 从数据数组构建线段树
 *
 * 递归自底向上构建三棵线段树（求和、最小、最大）。
 * 每个叶节点对应原数组的一个元素，
 * 每个内部节点存储其子节点区间的聚合值。
 *
 * @param data 输入数据数组
 */
void SegmentTree5::build(const QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();

    m_n = data.size();
    if (m_n == 0) return;

    /* 分配4n空间（线段树标准空间需求） */
    m_sumTree.resize(4 * m_n, 0.0);
    m_minTree.resize(4 * m_n, 0.0);
    m_maxTree.resize(4 * m_n, 0.0);

    /* 递归构建三棵树 */
    buildTree(m_sumTree, data, 1, 0, m_n - 1);
    buildTree(m_minTree, data, 1, 0, m_n - 1);
    buildTree(m_maxTree, data, 1, 0, m_n - 1);

    Q_UNUSED(timer);
}

/**
 * @brief 递归构建线段树
 *
 * @param tree 线段树数组
 * @param data 原始数据
 * @param node 当前节点索引（1为根）
 * @param lo 当前区间左边界
 * @param hi 当前区间右边界
 */
void SegmentTree5::buildTree(QVector<double>& tree, const QVector<double>& data,
                              int node, int lo, int hi)
{
    if (lo == hi) {
        /* 叶节点 */
        tree[node] = data[lo];
        return;
    }

    int mid = (lo + hi) / 2;
    buildTree(tree, data, 2 * node, lo, mid);
    buildTree(tree, data, 2 * node + 1, mid + 1, hi);

    /* 内部节点：聚合子节点 */
    if (&tree == &m_sumTree) {
        tree[node] = tree[2 * node] + tree[2 * node + 1];
    } else if (&tree == &m_minTree) {
        tree[node] = qMin(tree[2 * node], tree[2 * node + 1]);
    } else {
        tree[node] = qMax(tree[2 * node], tree[2 * node + 1]);
    }
}

/**
 * @brief 单点更新
 *
 * 更新指定位置的值，同时维护三棵线段树的一致性。
 * 自底向上更新所有包含该位置的祖先节点。
 *
 * @param pos 更新位置（0-indexed）
 * @param value 新值
 */
void SegmentTree5::update(int pos, double value)
{
    QElapsedTimer timer;
    timer.start();

    if (pos < 0 || pos >= m_n) return;

    updateTree(m_sumTree, 1, 0, m_n - 1, pos, value);
    updateTree(m_minTree, 1, 0, m_n - 1, pos, value);
    updateTree(m_maxTree, 1, 0, m_n - 1, pos, value);

    /* 更新统计 */
    m_stats.totalUpdates++;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    int total = m_stats.totalUpdates + m_stats.totalQueries;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit updated(pos);
}

/**
 * @brief 递归更新线段树节点
 *
 * @param tree 线段树数组
 * @param node 当前节点索引
 * @param lo 当前区间左边界
 * @param hi 当前区间右边界
 * @param pos 更新位置
 * @param val 新值
 */
void SegmentTree5::updateTree(QVector<double>& tree, int node, int lo, int hi,
                               int pos, double val)
{
    if (lo == hi) {
        tree[node] = val;
        return;
    }

    int mid = (lo + hi) / 2;
    if (pos <= mid)
        updateTree(tree, 2 * node, lo, mid, pos, val);
    else
        updateTree(tree, 2 * node + 1, mid + 1, hi, pos, val);

    /* 更新聚合值 */
    if (&tree == &m_sumTree) {
        tree[node] = tree[2 * node] + tree[2 * node + 1];
    } else if (&tree == &m_minTree) {
        tree[node] = qMin(tree[2 * node], tree[2 * node + 1]);
    } else {
        tree[node] = qMax(tree[2 * node], tree[2 * node + 1]);
    }
}

/**
 * @brief 区间求和查询
 * @param lo 区间左边界（包含）
 * @param hi 区间右边界（包含）
 * @return 区间[lo, hi]内所有元素的和
 */
double SegmentTree5::query(int lo, int hi) const
{
    QElapsedTimer timer;
    timer.start();

    if (lo < 0 || hi >= m_n || lo > hi) return 0.0;

    double result = querySum(1, 0, m_n - 1, lo, hi);

    const_cast<SegmentTree5*>(this)->m_stats.totalQueries++;
    double elapsed = timer.elapsed();
    const_cast<SegmentTree5*>(this)->m_timeSum += elapsed;
    int total = m_stats.totalUpdates + m_stats.totalQueries;
    const_cast<SegmentTree5*>(this)->m_stats.avgProcessingTimeMs =
        (total > 0) ? m_timeSum / total : 0.0;

    return result;
}

/**
 * @brief 区间最小值查询
 * @param lo 区间左边界（包含）
 * @param hi 区间右边界（包含）
 * @return 区间[lo, hi]内的最小值
 */
double SegmentTree5::queryMin(int lo, int hi) const
{
    QElapsedTimer timer;
    timer.start();

    if (lo < 0 || hi >= m_n || lo > hi) return 0.0;

    double result = SegmentTree5::queryMin(1, 0, m_n - 1, lo, hi);

    const_cast<SegmentTree5*>(this)->m_stats.totalQueries++;
    double elapsed = timer.elapsed();
    const_cast<SegmentTree5*>(this)->m_timeSum += elapsed;
    int total = m_stats.totalUpdates + m_stats.totalQueries;
    const_cast<SegmentTree5*>(this)->m_stats.avgProcessingTimeMs =
        (total > 0) ? m_timeSum / total : 0.0;

    return result;
}

/**
 * @brief 区间最大值查询
 * @param lo 区间左边界（包含）
 * @param hi 区间右边界（包含）
 * @return 区间[lo, hi]内的最大值
 */
double SegmentTree5::queryMax(int lo, int hi) const
{
    QElapsedTimer timer;
    timer.start();

    if (lo < 0 || hi >= m_n || lo > hi) return 0.0;

    double result = SegmentTree5::queryMax(1, 0, m_n - 1, lo, hi);

    const_cast<SegmentTree5*>(this)->m_stats.totalQueries++;
    double elapsed = timer.elapsed();
    const_cast<SegmentTree5*>(this)->m_timeSum += elapsed;
    int total = m_stats.totalUpdates + m_stats.totalQueries;
    const_cast<SegmentTree5*>(this)->m_stats.avgProcessingTimeMs =
        (total > 0) ? m_timeSum / total : 0.0;

    return result;
}

/**
 * @brief 递归区间求和查询
 * @param node 当前节点索引
 * @param lo 当前节点区间左边界
 * @param hi 当前节点区间右边界
 * @param ql 查询左边界
 * @param qr 查询右边界
 * @return 查询区间内的和
 */
double SegmentTree5::querySum(int node, int lo, int hi, int ql, int qr) const
{
    /* 查询区间与当前区间无交集 */
    if (ql > hi || qr < lo) return 0.0;

    /* 当前区间完全被查询区间包含 */
    if (ql <= lo && qr >= hi) return m_sumTree[node];

    /* 部分重叠，递归查询子区间 */
    int mid = (lo + hi) / 2;
    return querySum(2 * node, lo, mid, ql, qr) +
           querySum(2 * node + 1, mid + 1, hi, ql, qr);
}

/**
 * @brief 递归区间最小值查询
 * @param node 当前节点索引
 * @param lo 当前节点区间左边界
 * @param hi 当前节点区间右边界
 * @param ql 查询左边界
 * @param qr 查询右边界
 * @return 查询区间内的最小值
 */
double SegmentTree5::queryMin(int node, int lo, int hi, int ql, int qr) const
{
    if (ql > hi || qr < lo) return 1e18;
    if (ql <= lo && qr >= hi) return m_minTree[node];

    int mid = (lo + hi) / 2;
    return qMin(queryMin(2 * node, lo, mid, ql, qr),
                queryMin(2 * node + 1, mid + 1, hi, ql, qr));
}

/**
 * @brief 递归区间最大值查询
 * @param node 当前节点索引
 * @param lo 当前节点区间左边界
 * @param hi 当前节点区间右边界
 * @param ql 查询左边界
 * @param qr 查询右边界
 * @return 查询区间内的最大值
 */
double SegmentTree5::queryMax(int node, int lo, int hi, int ql, int qr) const
{
    if (ql > hi || qr < lo) return -1e18;
    if (ql <= lo && qr >= hi) return m_maxTree[node];

    int mid = (lo + hi) / 2;
    return qMax(queryMax(2 * node, lo, mid, ql, qr),
                queryMax(2 * node + 1, mid + 1, hi, ql, qr));
}

/**
 * @brief 重置所有统计数据
 */
void SegmentTree5::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}
