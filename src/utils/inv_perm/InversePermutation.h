/**
 * @file InversePermutation.h
 * @brief 置换群操作 — 逆置换/复合/阶/轮换分解
 *
 * 功能: 计算逆置换、置换复合、置换阶、轮换分解，
 *       统计操作次数/耗时。
 */
#pragma once

#include <QObject>
#include <QVector>

class InversePermutation : public QObject {
    Q_OBJECT
public:
    /** 统计信息 */
    struct Stats {
        quint64 totalOperations = 0;    ///< 总操作次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit InversePermutation(QObject* parent = nullptr);

    /**
     * @brief 计算逆置换
     * @param perm 原始置换(0-indexed)
     * @return 逆置换
     */
    QVector<int> inverse(const QVector<int>& perm);

    /**
     * @brief 复合两个置换 a(b(x))
     * @param a 第一个置换
     * @param b 第二个置换
     * @return 复合结果
     */
    QVector<int> compose(const QVector<int>& a, const QVector<int>& b);

    /**
     * @brief 计算置换的阶(最小正整数k使perm^k=identity)
     @param perm 置换
     * @return 阶
     */
    int order(const QVector<int>& perm);

    /**
     * @brief 将置换分解为不相交轮换
     * @param perm 置换
     * @return 轮换列表，每个轮换包含一组元素
     */
    QVector<QVector<int>> cycles(const QVector<int>& perm);

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 操作完成信号 */
    void operationCompleted();

private:
    Stats  m_stats;
    double m_timeSum;
};
