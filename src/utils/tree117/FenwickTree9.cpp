#include "FenwickTree9.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数
 * @param parent 父对象指针
 */
FenwickTree9::FenwickTree9(QObject* parent) : QObject(parent) {}

/**
 * @brief 重置所有统计信息
 */
void FenwickTree9::resetStatistics() { m_stats = Stats(); m_timeSum = 0.0; }

/* 内部树和原始数组 */
static QVector<long long> ftTree9;
static QVector<long long> ftOrig9;
static int ftSize9 = 0;

/**
 * @brief 初始化树状数组
 *
 * 从初始值序列构建Fenwick树，每个节点存储管辖区间之和。
 *
 * @param initialValues 初始值序列
 */
void FenwickTree9::build(const QVector<long long>& initialValues)
{
    QElapsedTimer timer;
    timer.start();

    ftSize9 = initialValues.size();
    ftOrig9 = initialValues;
    ftTree9.assign(ftSize9 + 1, 0);

    for (int i = 0; i < ftSize9; ++i) {
        int idx = i + 1;
        while (idx <= ftSize9) {
            ftTree9[idx] += initialValues[i];
            idx += idx & (-idx);
        }
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalQueryOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalQueryOps;
    emit queryCompleted(1);
}

/**
 * @brief 单点更新（将位置idx加上delta）
 * @param idx 更新位置（0-based）
 * @param delta 增量值
 */
void FenwickTree9::update(int idx, long long delta)
{
    QElapsedTimer timer;
    timer.start();

    if (idx < 0 || idx >= ftSize9) {
        emit queryCompleted(0);
        return;
    }

    ftOrig9[idx] += delta;
    int i = idx + 1;
    while (i <= ftSize9) {
        ftTree9[i] += delta;
        i += i & (-i);
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalQueryOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalQueryOps;
    emit queryCompleted(1);
}

/**
 * @brief 前缀和查询 [0, idx]
 * @param idx 查询终止位置
 * @return 前缀和
 */
long long FenwickTree9::prefixSum(int idx) const
{
    if (idx < 0 || idx >= ftSize9) return 0;
    long long sum = 0;
    int i = idx + 1;
    while (i > 0) {
        sum += ftTree9[i];
        i -= i & (-i);
    }
    return sum;
}

/**
 * @brief 范围求和查询 [left, right]
 * @param left 左边界
 * @param right 右边界
 * @return 范围和
 */
long long FenwickTree9::rangeSum(int left, int right) const
{
    if (left < 0 || right >= ftSize9 || left > right) return 0;
    if (left == 0) return prefixSum(right);
    return prefixSum(right) - prefixSum(left - 1);
}

/**
 * @brief 获取指定位置的值
 * @param idx 位置索引
 * @return 该位置的值
 */
long long FenwickTree9::valueAt(int idx) const
{
    if (idx < 0 || idx >= ftSize9) return 0;
    return ftOrig9[idx];
}
