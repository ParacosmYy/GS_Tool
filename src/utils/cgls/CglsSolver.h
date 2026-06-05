/**
 * @file CglsSolver.h
 * @brief 共轭梯度最小二乘求解器 — CGLS算法
 *
 * 功能: 求解最小二乘问题 min ||Ax - b||^2，基于CGLS迭代法，
 *       适用于超定/欠定线性系统，统计求解次数与平均耗时。
 *
 * 协作: QrDecomposition(分解法对比) / BicgstabSolver(非对称系统)
 */
#ifndef CGLSSOLVER_H
#define CGLSSOLVER_H

#include <QObject>
#include <QVector>

class CglsSolver : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalSolves    = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit CglsSolver(QObject* parent = nullptr);

    /**
     * @brief 求解最小二乘问题 min ||Ax - b||^2
     * @param matA 系数矩阵(m×n)
     * @param vecB 右侧向量(长度m)
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

#endif // CGLSSOLVER_H
