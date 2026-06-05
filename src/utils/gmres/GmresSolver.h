/**
 * @file GmresSolver.h
 * @brief GMRES求解器 — 广义最小残差法
 *
 * 功能: 求解一般非对称线性系统 Ax = b，基于带重启的GMRES迭代法，
 *       通过Arnoldi过程构建Krylov子空间，统计求解次数与平均耗时。
 *
 * 协作: BicgstabSolver(非对称备选) / MinresSolver(对称系统)
 */
#ifndef GMRESSOLVER_H
#define GMRESSOLVER_H

#include <QObject>
#include <QVector>

class GmresSolver : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalSolves    = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit GmresSolver(QObject* parent = nullptr);

    /**
     * @brief 求解一般线性系统 Ax = b (GMRES重启版本)
     * @param matA 系数矩阵(n×n)
     * @param vecB 右侧向量(长度n)
     * @param restart Krylov子空间重启维度(默认30)
     * @param tol  收敛容差(默认1e-10)
     * @param maxIter 最大迭代次数(默认1000)
     * @return 解向量x(长度n)
     */
    QVector<double> solve(const QVector<QVector<double>>& matA,
                          const QVector<double>& vecB,
                          int restart = 30,
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

#endif // GMRESSOLVER_H
