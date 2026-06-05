/**
 * @file FenwickTree3.cpp
 * @brief Fenwick树(树状数组)实现 — 高效前缀和与单点更新
 *
 * 支持O(log n)的前缀和查询和单点更新操作，
 * 以及区间和查询和二分搜索(下界查找)功能。
 * 使用1-based索引的经典Fenwick树结构。
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-05
 */

#include "utils/tree65/FenwickTree3.h"

#include <QElapsedTimer>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject指针
 */
FenwickTree3::FenwickTree3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 从数据构建Fenwick树
 *
 * 使用O(n)的构建方法: 先计算前缀和，
 * 再通过差值构建树状数组。
 *
 * @param data 输入数据序列
 */
void FenwickTree3::build(const QVector<double>& data)
{
    m_n = data.size();
    if (m_n == 0) return;

    /* Fenwick树使用1-based索引，数组大小为n+1 */
    m_tree.assign(m_n + 1, 0.0);

    /* O(n)构建: 先将数据放入树中 */
    for (int i = 0; i < m_n; ++i) {
        int treeIdx = i + 1; /* 转换为1-based */
        m_tree[treeIdx] += data[i];
        int parentIdx = treeIdx + lsb(treeIdx);
        if (parentIdx <= m_n) {
            m_tree[parentIdx] += m_tree[treeIdx];
        }
    }
}

/**
 * @brief 单点更新: 对位置idx的值增加delta
 *
 * 从idx向上更新所有受影响的树节点。
 * 时间复杂度: O(log n)
 *
 * @param idx 更新位置(0-based)，必须 >= 0 且 < size()
 * @param delta 增量值
 */
void FenwickTree3::update(int idx, double delta)
{
    QElapsedTimer timer;
    timer.start();

    if (idx < 0 || idx >= m_n) return;

    /* 从idx+1(1-based)向上更新 */
    int i = idx + 1;
    while (i <= m_n) {
        m_tree[i] += delta;
        i += lsb(i);
    }

    /* 更新统计信息 */
    m_stats.totalUpdates++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalUpdates + m_stats.totalQueries);

    emit updated(idx, delta);
}

/**
 * @brief 前缀和查询: 计算[0, idx]范围内元素之和
 *
 * 从idx向下累加树节点的值。
 * 时间复杂度: O(log n)
 *
 * @param idx 查询右端点(0-based, 包含)
 * @return [0, idx]范围内元素之和
 */
double FenwickTree3::prefixSum(int idx) const
{
    QElapsedTimer timer;
    timer.start();

    if (idx < 0 || m_n == 0) return 0.0;
    idx = qMin(idx, m_n - 1);

    double sum = 0.0;
    int i = idx + 1; /* 转换为1-based */
    while (i > 0) {
        sum += m_tree[i];
        i -= lsb(i);
    }

    const_cast<FenwickTree3*>(this)->m_stats.totalQueries++;
    const_cast<FenwickTree3*>(this)->m_timeSum += timer.elapsed();
    const_cast<FenwickTree3*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / (m_stats.totalUpdates + m_stats.totalQueries);

    return sum;
}

/**
 * @brief 区间和查询: 计算[lo, hi]范围内元素之和
 *
 * 利用前缀和之差: rangeSum(lo, hi) = prefixSum(hi) - prefixSum(lo-1)
 *
 * @param lo 区间左端点(0-based, 包含)
 * @param hi 区间右端点(0-based, 包含)
 * @return 区间元素之和
 */
double FenwickTree3::rangeSum(int lo, int hi) const
{
    if (lo > hi || lo < 0 || hi >= m_n) return 0.0;
    if (lo == 0) return prefixSum(hi);
    return prefixSum(hi) - prefixSum(lo - 1);
}

/**
 * @brief 二分搜索下界: 找到前缀和首次 >= target 的最小索引
 *
 * 使用Fenwick树的二分搜索技巧，从最高位到最低位逐位确定位置。
 * 时间复杂度: O(log n)
 *
 * @param target 目标值
 * @return 前缀和 >= target 的最小索引(0-based)，不存在返回n
 */
int FenwickTree3::lowerBound(double target) const
{
    QElapsedTimer timer;
    timer.start();

    if (m_n == 0 || target <= 0.0) {
        const_cast<FenwickTree3*>(this)->m_stats.totalQueries++;
        const_cast<FenwickTree3*>(this)->m_timeSum += timer.elapsed();
        const_cast<FenwickTree3*>(this)->m_stats.avgProcessingTimeMs =
            m_timeSum / (m_stats.totalUpdates + m_stats.totalQueries);
        return (target <= 0.0) ? 0 : m_n;
    }

    /* 找到最大的2的幂 <= n */
    int pos = 0;
    int bitMask = 1;
    while (bitMask * 2 <= m_n) {
        bitMask *= 2;
    }

    double sum = 0.0;
    /* 从高位到低位逐位构造答案 */
    for (int step = bitMask; step > 0; step /= 2) {
        int nextPos = pos + step;
        if (nextPos <= m_n && sum + m_tree[nextPos] < target) {
            sum += m_tree[nextPos];
            pos = nextPos;
        }
    }

    int result = pos; /* 0-based索引 */
    if (pos < m_n) {
        result = pos; /* 返回的是前缀和首次 >= target 的索引 */
    } else {
        result = m_n; /* 所有元素之和 < target */
    }

    const_cast<FenwickTree3*>(this)->m_stats.totalQueries++;
    const_cast<FenwickTree3*>(this)->m_timeSum += timer.elapsed();
    const_cast<FenwickTree3*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / (m_stats.totalUpdates + m_stats.totalQueries);

    return result;
}

/**
 * @brief 重置所有统计数据
 */
void FenwickTree3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 获取指定位置的当前值
 *
 * 通过两个前缀和之差获取单点值: value[i] = prefixSum(i) - prefixSum(i-1)
 *
 * @param idx 位置索引(0-based)
 * @return 该位置的当前值
 */
double FenwickTree3::valueAt(int idx) const
{
    if (idx < 0 || idx >= m_n) return 0.0;
    if (idx == 0) return prefixSum(0);
    return prefixSum(idx) - prefixSum(idx - 1);
}

/**
 * @brief 获取所有元素的当前值
 *
 * 逐一计算每个位置的前缀和差值，重建完整的数值数组。
 * 时间复杂度: O(n log n)
 *
 * @return 包含所有当前值的向量
 */
QVector<double> FenwickTree3::toVector() const
{
    if (m_n == 0) return {};

    QVector<double> result(m_n);
    double prevSum = 0.0;
    for (int i = 0; i < m_n; ++i) {
        double curSum = prefixSum(i);
        result[i] = curSum - prevSum;
        prevSum = curSum;
    }
    return result;
}

/**
 * @brief 计算所有元素的总和
 *
 * 等价于prefixSum(n-1)。
 *
 * @return 所有元素之和
 */
double FenwickTree3::totalSum() const
{
    if (m_n == 0) return 0.0;
    return prefixSum(m_n - 1);
}

/**
 * @brief 查找值等于指定目标的第一个位置
 *
 * 结合前缀和和二分搜索，找到值恰好等于target的位置。
 * 如果没有精确匹配，返回最接近的位置。
 *
 * @param target 目标值
 * @return 最接近target的位置索引(0-based)
 */
int FenwickTree3::findClosest(double target) const
{
    if (m_n == 0) return -1;

    /* 使用lowerBound找到第一个 >= target 的位置 */
    int lb = lowerBound(target);

    if (lb == 0) return 0;
    if (lb >= m_n) return m_n - 1;

    /* 比较lb和lb-1的前缀和，返回更接近target的 */
    double sumLb = prefixSum(lb);
    double sumLb1 = prefixSum(lb - 1);

    if (qAbs(sumLb - target) < qAbs(sumLb1 - target)) {
        return lb;
    }
    return lb - 1;
}

/**
 * @brief 计算树中非零元素的数量
 *
 * 遍历所有位置，统计值不为0的元素个数。
 *
 * @return 非零元素数量
 */
int FenwickTree3::nonZeroCount() const
{
    if (m_n == 0) return 0;

    int count = 0;
    double prevSum = 0.0;
    for (int i = 0; i < m_n; ++i) {
        double curSum = prefixSum(i);
        if (qAbs(curSum - prevSum) > 1e-15) {
            ++count;
        }
        prevSum = curSum;
    }
    return count;
}
