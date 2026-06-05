/**
 * @file SorSolver.h
 * @brief SOR迭代求解器 — 逐次超松弛
 *
 * 功能: 使用SOR(Successive Over-Relaxation)迭代法求解线性系统，
 *       通过松弛因子omega加速收敛。
 *
 * 协作: JacobiSolver(Jacobi迭代) / GaussSeidelSolver(Gauss-Seidel)
 */
#ifndef SORSOLVER_H
#define SORSOLVER_H

#include <QObject>
#include <QVector>

/**
 * @brief SOR迭代求解器
 */
class SorSolver : public QObject {
    Q_OBJECT

public:
    /** @brief 统计 */
    struct Stats {
        quint64 totalSolves = 0;            ///< 累计求解次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    explicit SorSolver(QObject* parent = nullptr);

    /**
     * @brief SOR迭代求解
     * @param A 系数矩阵
     * @param b 右端向量
     * @param omega 松弛因子(1.0~2.0)
     * @param tol 收敛容差
     * @param maxIter 最大迭代次数
     * @return 解向量
     */
    QVector<double> solve(const QVector<QVector<double>>& A,
                           const QVector<double>& b,
                           double omega = 1.5,
                           double tol = 1e-10, int maxIter = 1000);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 求解完成 @param iterations 迭代次数 @param residual 最终残差 */
    void solveCompleted(int iterations, double residual);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
};

#endif // SORSOLVER_H
