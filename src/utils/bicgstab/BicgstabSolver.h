/**
 * @file BicgstabSolver.h
 * @brief BiCGSTAB求解器 — 稳定双共轭梯度法
 *
 * 功能: 求解一般非对称线性系统 Ax = b，基于BiCGSTAB迭代法，
 *       适用于非对称/不定矩阵，统计求解次数与平均耗时。
 *
 * 协作: CglsSolver(最小二乘) / GmresSolver(GMRES)
 */
#ifndef BICGSTABSOLVER_H
#define BICGSTABSOLVER_H

#include <QObject>
#include <QVector>

class BicgstabSolver : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalSolves    = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit BicgstabSolver(QObject* parent = nullptr);

    /**
     * @brief 求解非对称线性系统 Ax = b
     * @param matA 系数矩阵(n×n)
     * @param vecB 右侧向量(长度n)
     * @param tol  收敛容差(默认1e-10)
     * @param maxIter 最大迭代次数(默认1000)
     * @return 解向量x(长度n)
     */
    QVector<double> solve(const QVector<QVector<double>>& matA,
                          const QVector<double>& vecB,
                          double tol = 1e-10,
                          int maxIter = 1000);

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 求解完成信号 @param iterations 迭代次数 @param residual 残差 */
    void solveCompleted(int iterations, double residual);

private:
    Stats  m_stats;
    double m_timeSum;
};

#endif // BICGSTABSOLVER_H
