#include "FenwickTree7.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @file FenwickTree7.cpp
 * @brief 树状数组(Binary Indexed Tree)实现
 *
 * 基于Fenwick树实现O(logN)的单点更新和前缀和查询。
 * 核心思想: 利用整数二进制表示中最低位的1来划分区间。
 */

/**
 * @brief 构造函数，初始化空树
 * @param parent 父QObject对象指针
 */
FenwickTree7::FenwickTree7(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置数组大小
 * @param n 数组的大小(1-indexed)
 */
void FenwickTree7::setSize(int n)
{
    m_tree.assign(n + 1, 0.0); // Fenwick树使用1-indexed
}

/**
 * @brief 计算最低位1的值(lowbit)
 * @param x 输入整数
 * @return x的最低位1对应的值
 */
static inline int lowbit(int x)
{
    return x & (-x);
}

/**
 * @brief 单点更新: 位置idx增加delta
 *
 * 从idx开始，逐级向上更新父节点:
 * 父节点索引 = 当前索引 + lowbit(当前索引)
 *
 * @param idx 更新位置(1-indexed)
 * @param delta 增量值
 */
void FenwickTree7::update(int idx, double delta)
{
    if (idx < 1 || idx >= m_tree.size()) return;

    QElapsedTimer timer;
    timer.start();

    // 从当前位置向上更新所有覆盖该位置的区间
    while (idx < m_tree.size()) {
        m_tree[idx] += delta;
        idx += lowbit(idx);
    }

    m_stats.totalUpdates++;
    m_timeSum += timer.elapsed();
    const int total = m_stats.totalUpdates + m_stats.totalQueries;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit updated(idx, delta);
}

/**
 * @brief 计算前缀和[1, idx]
 * @param idx 前缀和的右端点(1-indexed)
 * @return 前缀和值
 */
double FenwickTree7::prefixSum(int idx)
{
    double sum = 0.0;
    while (idx > 0) {
        sum += m_tree[idx];
        idx -= lowbit(idx);
    }
    return sum;
}

/**
 * @brief 区间查询: [left, right]的和
 *
 * 利用前缀和之差计算区间和:
 * sum(left, right) = prefixSum(right) - prefixSum(left - 1)
 *
 * @param left 区间左端点(1-indexed)
 * @param right 区间右端点(1-indexed)
 * @return 区间[left, right]内的元素和
 */
double FenwickTree7::query(int left, int right)
{
    if (left > right || left < 1 || right >= m_tree.size()) return 0.0;

    QElapsedTimer timer;
    timer.start();

    const double result = prefixSum(right) - prefixSum(left - 1);

    m_stats.totalQueries++;
    m_timeSum += timer.elapsed();
    const int total = m_stats.totalUpdates + m_stats.totalQueries;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    return result;
}

/**
 * @brief 重置所有统计信息
 */
void FenwickTree7::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_tree.clear();
}
