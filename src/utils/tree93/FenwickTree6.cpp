#include "FenwickTree6.h"
#include <QElapsedTimer>
#include <algorithm>

/**
 * @brief 构造函数，初始化树状数组
 * @param parent 父对象指针
 */
FenwickTree6::FenwickTree6(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置数组大小并初始化
 * @param size 数组大小
 */
void FenwickTree6::setSize(int size)
{
    m_size = qMax(0, size);
    m_tree.assign(m_size + 1, 0.0);
}

/**
 * @brief 单点更新：将位置index增加delta
 *
 * Fenwick树更新操作：从index+1开始，逐级向上更新父节点。
 * 父节点索引 = i + (i & (-i))
 * 时间复杂度 O(log n)。
 *
 * @param index 更新位置(0-based)
 * @param delta 增量值
 */
void FenwickTree6::update(int index, double delta)
{
    QElapsedTimer timer;
    timer.start();

    if (index < 0 || index >= m_size) {
        m_timeSum += timer.elapsed();
        m_stats.totalOperations++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
        return;
    }

    /* Fenwick树使用1-based索引 */
    int i = index + 1;
    while (i <= m_size) {
        m_tree[i] += delta;
        i += (i & (-i)); /* 移动到下一个父节点 */
    }

    m_timeSum += timer.elapsed();
    m_stats.totalOperations++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
    emit updated(index, delta);
}

/**
 * @brief 内部前缀和查询：返回[0, index]的累积和
 * @param index 终止位置(0-based)
 * @return 前缀和
 */
double FenwickTree6::prefixSum(int index) const
{
    double sum = 0.0;
    int i = index + 1;
    while (i > 0) {
        sum += m_tree[i];
        i -= (i & (-i)); /* 移动到前一个覆盖区间 */
    }
    return sum;
}

/**
 * @brief 区间查询：返回[left, right]的和
 *
 * 利用前缀和之差计算区间和：sum(right) - sum(left-1)。
 * 时间复杂度 O(log n)。
 *
 * @param left 区间左端点(0-based, 包含)
 * @param right 区间右端点(0-based, 包含)
 * @return 区间和
 */
double FenwickTree6::query(int left, int right)
{
    QElapsedTimer timer;
    timer.start();

    if (left < 0 || right >= m_size || left > right) {
        m_timeSum += timer.elapsed();
        m_stats.totalOperations++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
        return 0.0;
    }

    double result = prefixSum(right);
    if (left > 0) result -= prefixSum(left - 1);

    m_timeSum += timer.elapsed();
    m_stats.totalOperations++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
    return result;
}

/**
 * @brief 单点查询：获取指定位置的值
 * @param index 查询位置(0-based)
 * @return 该位置的累积值
 */
double FenwickTree6::pointQuery(int index) const
{
    if (index < 0 || index >= m_size) return 0.0;
    return prefixSum(index) - (index > 0 ? prefixSum(index - 1) : 0.0);
}

/**
 * @brief 批量构建树状数组
 *
 * 从初始数组高效构建Fenwick树，时间复杂度O(n)。
 *
 * @param values 初始数组值
 */
void FenwickTree6::build(const QVector<double>& values)
{
    m_size = values.size();
    m_tree.assign(m_size + 1, 0.0);
    for (int i = 0; i < m_size; ++i) {
        update(i, values[i]);
    }
}

/**
 * @brief 获取数组大小
 * @return 数组大小
 */
int FenwickTree6::size() const
{
    return m_size;
}

/**
 * @brief 重置统计数据
 */
void FenwickTree6::resetStatistics()
{
    m_stats.totalOperations = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
    m_tree.assign(m_size + 1, 0.0);
}
