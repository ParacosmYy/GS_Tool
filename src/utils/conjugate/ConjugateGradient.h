/**
 * @file ConjugateGradient.h
 * @brief 共轭梯度法 — 求解对称正定线性系统 Ax=b
 *
 * 功能: 使用共轭梯度(CG)迭代求解稀疏对称正定线性方程组，
 *       支持残差监控和迭代终止控制。
 *
 * 协作: LuDecomposition(直接法) / CholeskyDecomposition(直接法)
 */
#ifndef CONJUGATEGRADIENT_H
#define CONJUGATEGRADIENT_H

#include <QObject>
#include <QVector>

/**
 * @brief 共轭梯度法求解器
 */
class ConjugateGradient : public QObject {
    Q_OBJECT

public:
    /** @brief 统计 */
    struct Stats {
        quint64 totalSolves = 0;            ///< 累计求解次数
        quint64 totalIterations = 0;        ///< 累计迭代次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    explicit ConjugateGradient(QObject* parent = nullptr);

    /**
     * @brief 求解对称正定线性系统 Ax = b
     * @param A 对称正定矩阵(n×n)
     * @param b 右端向量(n)
     * @param maxIter 最大迭代次数
     * @param tol 收敛容差
     * @return 解向量x
     */
    QVector<double> solve(const QVector<QVector<double>>& A,
                          const QVector<double>& b,
                          int maxIter = 1000,
                          double tol = 1e-10);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 求解完成 @param iterations 迭代次数 @param residual 最终残差 */
    void solveCompleted(int iterations, double residual);

private:
    /** @brief 向量内积 */
    double dotProduct(const QVector<double>& a,
                      const QVector<double>& b) const;

    /** @brief 矩阵-向量乘法 */
    QVector<double> matVecMul(const QVector<QVector<double>>& A,
                               const QVector<double>& x) const;

    Stats m_stats;              ///< 统计信息
    double m_timeSum = 0.0;     ///< 累计耗时
};

#endif // CONJUGATEGRADIENT_H
