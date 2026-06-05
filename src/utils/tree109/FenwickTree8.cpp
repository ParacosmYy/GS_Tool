#include "FenwickTree8.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化Fenwick树
 * @param parent 父对象指针
 */
FenwickTree8::FenwickTree8(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void FenwickTree8::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 初始化Fenwick树
 *
 * 从初始值数组构建树状数组，每个节点存储其管辖区间的和。
 * 时间复杂度O(n log n)。
 *
 * @param initialValues 初始值数组
 */
void FenwickTree8::initialize(const QVector<double>& initialValues)
{
    QElapsedTimer timer;
    timer.start();

    m_size = initialValues.size();
    m_original = initialValues;
    m_tree.assign(m_size + 1, 0.0);

    for (int i = 0; i < m_size; ++i) {
        int idx = i + 1;
        while (idx <= m_size) {
            m_tree[idx] += initialValues[i];
            idx += idx & (-idx);
        }
    }

    m_stats.treeSize = m_size;

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalOperations++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit operationCompleted(QStringLiteral("initialize"));
}

/**
 * @brief 单点更新（增加增量）
 *
 * 将指定位置增加delta，同时更新所有受影响的Fenwick节点。
 *
 * @param index 目标索引（0-based）
 * @param delta 增量值
 */
void FenwickTree8::update(int index, double delta)
{
    QElapsedTimer timer;
    timer.start();

    if (index < 0 || index >= m_size) return;

    m_original[index] += delta;
    int idx = index + 1;
    while (idx <= m_size) {
        m_tree[idx] += delta;
        idx += idx & (-idx);
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalOperations++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit operationCompleted(QStringLiteral("update"));
}

/**
 * @brief 前缀和查询 [0, index]
 *
 * 累加从位置0到index的所有值，时间复杂度O(log n)。
 *
 * @param index 查询右端点
 * @return 前缀和
 */
double FenwickTree8::prefixSum(int index) const
{
    if (index < 0 || index >= m_size) return 0.0;

    double sum = 0.0;
    int idx = index + 1;
    while (idx > 0) {
        sum += m_tree[idx];
        idx -= idx & (-idx);
    }
    return sum;
}

/**
 * @brief 区间和查询 [left, right]
 *
 * 利用前缀和之差计算区间和。
 *
 * @param left 左端点
 * @param right 右端点
 * @return 区间和
 */
double FenwickTree8::rangeSum(int left, int right) const
{
    if (left < 0 || right >= m_size || left > right) return 0.0;
    if (left == 0) return prefixSum(right);
    return prefixSum(right) - prefixSum(left - 1);
}

/**
 * @brief 获取原始数组值
 *
 * @param index 目标索引
 * @return 当前值
 */
double FenwickTree8::valueAt(int index) const
{
    if (index < 0 || index >= m_size) return 0.0;
    return m_original[index];
}
