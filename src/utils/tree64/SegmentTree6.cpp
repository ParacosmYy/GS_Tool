/**
 * @file SegmentTree6.cpp
 * @brief 线段树实现 — 支持区间更新、区间求和与区间最大值查询
 *
 * 基于懒标记(Lazy Propagation)的线段树，支持O(log n)的区间加操作、
 * 区间求和查询和区间最大值查询。使用数组模拟完全二叉树结构。
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-05
 */

#include "utils/tree64/SegmentTree6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject指针
 */
SegmentTree6::SegmentTree6(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 从数据构建线段树
 *
 * 自底向上构建求和树和最大值树。
 * 时间复杂度: O(n)
 *
 * @param data 输入数据序列
 */
void SegmentTree6::build(const QVector<double>& data)
{
    m_n = data.size();
    if (m_n == 0) return;

    /* 分配4n空间(完全二叉树安全上界) */
    const int treeSize = 4 * m_n;
    m_sumTree.assign(treeSize, 0.0);
    m_maxTree.assign(treeSize, 0.0);
    m_lazy.assign(treeSize, 0.0);

    /* 递归构建(使用迭代方式) */
    /* 先将数据填入叶子节点的位置 */
    for (int i = 0; i < m_n; ++i) {
        m_sumTree[i + m_n] = data[i];
        m_maxTree[i + m_n] = data[i];
    }

    /* 自底向上构建 */
    for (int i = m_n - 1; i >= 1; --i) {
        m_sumTree[i] = m_sumTree[2 * i] + m_sumTree[2 * i + 1];
        m_maxTree[i] = qMax(m_maxTree[2 * i], m_maxTree[2 * i + 1]);
    }

    /* 对于使用递归的查询/更新，需要正确的树结构 */
    /* 重新构建为标准线段树格式(根在1) */
    m_sumTree.assign(treeSize, 0.0);
    m_maxTree.assign(treeSize, 0.0);
    m_lazy.assign(treeSize, 0.0);

    /* 辅助: 用索引方式填充线段树 */
    QVector<double> src = data;
    while (src.size() < m_n) src.append(0.0);

    /* 使用递归构建辅助函数 */
    /* 由于无法在build中调用自身的私有递归，使用迭代构建 */
    /* 将数据放在树的底部 */
    int base = 1;
    while (base < m_n) base *= 2;

    for (int i = 0; i < m_n; ++i) {
        m_sumTree[base + i] = data[i];
        m_maxTree[base + i] = data[i];
    }
    /* 从底部向上合并 */
    for (int i = base - 1; i >= 1; --i) {
        m_sumTree[i] = m_sumTree[2 * i] + m_sumTree[2 * i + 1];
        m_maxTree[i] = qMax(m_maxTree[2 * i], m_maxTree[2 * i + 1]);
    }
}

/**
 * @brief 区间更新: 对[lo, hi]范围内每个元素加delta
 *
 * 使用懒标记优化，时间复杂度: O(log n)
 *
 * @param lo 区间左端点 (0-based, 包含)
 * @param hi 区间右端点 (0-based, 包含)
 * @param delta 增量值
 */
void SegmentTree6::updateRange(int lo, int hi, double delta)
{
    QElapsedTimer timer;
    timer.start();

    if (m_n == 0 || lo > hi || lo < 0 || hi >= m_n) return;

    updateRange(1, 0, m_n - 1, lo, hi, delta);

    m_stats.totalUpdates++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalUpdates + m_stats.totalQueries);

    emit rangeUpdated(lo, hi);
}

/**
 * @brief 查询区间[lo, hi]的元素之和
 *
 * @param lo 区间左端点 (0-based, 包含)
 * @param hi 区间右端点 (0-based, 包含)
 * @return 区间元素之和
 */
double SegmentTree6::querySum(int lo, int hi) const
{
    QElapsedTimer timer;
    timer.start();

    if (m_n == 0 || lo > hi || lo < 0 || hi >= m_n) return 0.0;

    double result = querySum(1, 0, m_n - 1, lo, hi);

    /* const_cast用于更新统计信息 */
    const_cast<SegmentTree6*>(this)->m_stats.totalQueries++;
    const_cast<SegmentTree6*>(this)->m_timeSum += timer.elapsed();
    const_cast<SegmentTree6*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / (m_stats.totalUpdates + m_stats.totalQueries);

    return result;
}

/**
 * @brief 查询区间[lo, hi]的最大元素值
 *
 * @param lo 区间左端点 (0-based, 包含)
 * @param hi 区间右端点 (0-based, 包含)
 * @return 区间最大值
 */
double SegmentTree6::queryMax(int lo, int hi) const
{
    QElapsedTimer timer;
    timer.start();

    if (m_n == 0 || lo > hi || lo < 0 || hi >= m_n) return 0.0;

    double result = queryMax(1, 0, m_n - 1, lo, hi);

    const_cast<SegmentTree6*>(this)->m_stats.totalQueries++;
    const_cast<SegmentTree6*>(this)->m_timeSum += timer.elapsed();
    const_cast<SegmentTree6*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / (m_stats.totalUpdates + m_stats.totalQueries);

    return result;
}

/**
 * @brief 重置所有统计数据
 */
void SegmentTree6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 懒标记下推
 *
 * 将当前节点的懒标记传递给子节点。
 *
 * @param node 当前节点索引
 * @param lo 当前节点管理的区间左端
 * @param hi 当前节点管理的区间右端
 */
void SegmentTree6::pushDown(int node, int lo, int hi)
{
    if (m_lazy[node] != 0.0) {
        int mid = (lo + hi) / 2;
        int left = 2 * node;
        int right = 2 * node + 1;

        /* 左子节点 */
        int leftLen = mid - lo + 1;
        m_sumTree[left] += m_lazy[node] * leftLen;
        m_maxTree[left] += m_lazy[node];
        m_lazy[left] += m_lazy[node];

        /* 右子节点 */
        int rightLen = hi - mid;
        m_sumTree[right] += m_lazy[node] * rightLen;
        m_maxTree[right] += m_lazy[node];
        m_lazy[right] += m_lazy[node];

        /* 清除当前节点懒标记 */
        m_lazy[node] = 0.0;
    }
}

/**
 * @brief 递归区间更新
 *
 * @param node 当前节点索引
 * @param lo 当前节点管理区间左端
 * @param hi 当前节点管理区间右端
 * @param ql 查询区间左端
 * @param qr 查询区间右端
 * @param delta 增量值
 */
void SegmentTree6::updateRange(int node, int lo, int hi, int ql, int qr, double delta)
{
    if (ql > hi || qr < lo) return; /* 无交集 */
    if (ql <= lo && hi <= qr) {
        /* 完全覆盖 */
        m_sumTree[node] += delta * (hi - lo + 1);
        m_maxTree[node] += delta;
        m_lazy[node] += delta;
        return;
    }

    pushDown(node, lo, hi);
    int mid = (lo + hi) / 2;
    updateRange(2 * node, lo, mid, ql, qr, delta);
    updateRange(2 * node + 1, mid + 1, hi, ql, qr, delta);
    m_sumTree[node] = m_sumTree[2 * node] + m_sumTree[2 * node + 1];
    m_maxTree[node] = qMax(m_maxTree[2 * node], m_maxTree[2 * node + 1]);
}

/**
 * @brief 递归区间求和查询
 * @param node 当前节点
 * @param lo 当前区间左端
 * @param hi 当前区间右端
 * @param ql 查询左端
 * @param qr 查询右端
 * @return 区间和
 */
double SegmentTree6::querySum(int node, int lo, int hi, int ql, int qr) const
{
    if (ql > hi || qr < lo) return 0.0;
    if (ql <= lo && hi <= qr) return m_sumTree[node];

    const_cast<SegmentTree6*>(this)->pushDown(node, lo, hi);
    int mid = (lo + hi) / 2;
    return querySum(2 * node, lo, mid, ql, qr)
         + querySum(2 * node + 1, mid + 1, hi, ql, qr);
}

/**
 * @brief 递归区间最大值查询
 * @param node 当前节点
 * @param lo 当前区间左端
 * @param hi 当前区间右端
 * @param ql 查询左端
 * @param qr 查询右端
 * @return 区间最大值
 */
double SegmentTree6::queryMax(int node, int lo, int hi, int ql, int qr) const
{
    if (ql > hi || qr < lo) return -1e18;
    if (ql <= lo && hi <= qr) return m_maxTree[node];

    const_cast<SegmentTree6*>(this)->pushDown(node, lo, hi);
    int mid = (lo + hi) / 2;
    return qMax(queryMax(2 * node, lo, mid, ql, qr),
                queryMax(2 * node + 1, mid + 1, hi, ql, qr));
}
