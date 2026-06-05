/**
 * @file VandermondeSolver.h
 * @brief Vandermonde矩阵求解器 — 多项式插值系数
 *
 * 功能: 求解Vandermonde系统以获取多项式插值系数，
 *       使用Björck-Pereyra算法(O(n^2))。
 *
 * 协作: LagrangeInterpolation(Lagrange插值) / DividedDifference(差商)
 */
#ifndef VANDERMONDESOLVER_H
#define VANDERMONDESOLVER_H

#include <QObject>
#include <QVector>

/**
 * @brief Vandermonde矩阵求解器
 */
class VandermondeSolver : public QObject {
    Q_OBJECT

public:
    /** @brief 统计 */
    struct Stats {
        quint64 totalSolves = 0;            ///< 累计求解次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    explicit VandermondeSolver(QObject* parent = nullptr);

    /**
     * @brief 求解Vandermonde系统 V*coeffs = values
     * @param nodes 节点向量
     * @param values 节点值向量
     * @return 多项式系数(升幂排列)
     */
    QVector<double> solve(const QVector<double>& nodes,
                           const QVector<double>& values);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 求解完成 @param size 矩阵维度 */
    void solveCompleted(int size);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
};

#endif // VANDERMONDESOLVER_H
