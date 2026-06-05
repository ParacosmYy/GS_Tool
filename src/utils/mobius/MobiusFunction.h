/**
 * @file MobiusFunction.h
 * @brief Mobius函数与欧拉函数 — 数论工具
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @class MobiusFunction
 * @brief 计算Mobius函数 mu(n) 和欧拉函数 phi(n)
 *
 * Mobius函数: mu(1)=1, 若n含平方因子则为0, 否则 (-1)^k (k为质因子个数)
 * 欧拉函数: phi(n) = 小于n且与n互质的正整数个数
 */
class MobiusFunction : public QObject {
    Q_OBJECT
public:
    /** @brief 统计信息结构体 */
    struct Stats {
        quint64 totalComputed = 0;          ///< 总计算次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理时间(ms)
    };

    explicit MobiusFunction(QObject* parent = nullptr);

    /**
     * @brief 计算单个n的Mobius函数值
     * @param n 正整数 (n >= 1)
     * @return mu(n): -1, 0, 或 1
     */
    int compute(int n);

    /**
     * @brief 批量筛法计算Mobius函数 [1, maxN]
     * @param maxN 筛的上界 (>= 1)
     * @return mu[1..maxN] 数组
     */
    QVector<int> sieve(int maxN);

    /**
     * @brief 计算单个n的欧拉函数值
     * @param n 正整数 (n >= 1)
     * @return phi(n)
     */
    int eulerTotient(int n);

    /**
     * @brief 批量筛法计算欧拉函数 [1, maxN]
     * @param maxN 筛的上界 (>= 1)
     * @return phi[1..maxN] 数组
     */
    QVector<int> totientSieve(int maxN);

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 筛法完成信号 @param maxN 筛上界 */
    void sieveCompleted(int maxN);

private:
    mutable Stats  m_stats;
    mutable double m_timeSum = 0.0;
};
