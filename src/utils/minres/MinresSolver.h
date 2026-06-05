/**
 * @file MinresSolver.h
 * @brief MINRES求解器 — 最小残差法
 *
 * 功能: 求解对称不定线性系统 Ax = b，基于MINRES迭代法，
 *       利用Lanczos三对角化，统计求解次数与平均耗时。
 *
 * 协作: GmresSolver(非对称扩展) / SymmetricEigenSolver(特征值)
 */
#ifndef MINRESSOLVER_H
#define MINRESSOLVER_H

#include <QObject>
#include <QVector>

class MinresSolver : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalSolves    = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit MinresSolver(QObject* parent = nullptr);

    /**
     * @brief 求解对称不定线性系统 Ax = b
     * @param matA 对称系数矩阵(n×n)
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

#endif // MINRESSOLVER_H
