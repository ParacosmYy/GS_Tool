/**
 * @file JacobiSolver.h
 * @brief Jacobi迭代求解器 — 经典迭代法
 *
 * 功能: 使用Jacobi迭代法求解线性系统 Ax=b，
 *       每步用上一轮全部值并行更新。
 *
 * 协作: SorSolver(SOR加速) / GaussSeidelSolver(Gauss-Seidel)
 */
#ifndef JACOBISOLVER_H
#define JACOBISOLVER_H

#include <QObject>
#include <QVector>

/**
 * @brief Jacobi迭代求解器
 */
class JacobiSolver : public QObject {
    Q_OBJECT

public:
    /** @brief 统计 */
    struct Stats {
        quint64 totalSolves = 0;            ///< 累计求解次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    explicit JacobiSolver(QObject* parent = nullptr);

    /**
     * @brief Jacobi迭代求解
     * @param A 系数矩阵
     * @param b 右端向量
     * @param tol 收敛容差
     * @param maxIter 最大迭代次数
     * @return 解向量
     */
    QVector<double> solve(const QVector<QVector<double>>& A,
                           const QVector<double>& b,
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

#endif // JACOBISOLVER_H
