/**
 * @file GaussSeidel5.h
 * @brief 高斯-赛德尔迭代(逐次超松弛+红黑排序并行稀疏求解) — Gauss-Seidel with Successive Over-Relaxation and Red-Black Ordering for Parallelizable Sparse System Solving
 *
 * 功能: 实现高斯-赛德尔迭代(Gauss-Seidel iteration)配合逐次超松弛(successive
 *       over-relaxation, SOR)加速收敛，采用红黑排序(red-black ordering)将未知量
 *       分为两色集交替更新，提升并行化友好度，适用于大规模稀疏线性系统求解。
 *
 * 协作: GaussElimination4(高斯消元) / LUDecomposition3(LU分解) / SparseMatrix2(稀疏矩阵)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 高斯-赛德尔迭代(逐次超松弛+红黑排序并行稀疏求解)
 */
class GaussSeidel5 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int matrixSize = 0;
        int numIterations = 0;
        double finalResidual = 0.0;
        double omega = 1.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GaussSeidel5(QObject *parent = nullptr);
    ~GaussSeidel5() override;

    /** @brief Set relaxation factor omega (0 < omega < 2) */
    void setOmega(double omega);

    /** @brief Set convergence tolerance */
    void setTolerance(double tol);

    /** @brief Set maximum iterations */
    void setMaxIterations(int maxIter);

    /** @brief Solve Ax = b. A is [n x n], b is [n] */
    QVector<double> solve(const QVector<QVector<double>>& A,
                          const QVector<double>& b);

    /** @brief Solve using red-black ordering for better parallelism */
    QVector<double> solveRedBlack(const QVector<QVector<double>>& A,
                                  const QVector<double>& b);

    /** @brief Get residual history */
    QVector<double> residualHistory() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void iterationCompleted(int iter, double residual);
    void solveCompleted(int iterations, double finalResidual, double timeMs);

private:
    double m_omega = 1.5;
    double m_tolerance = 1e-10;
    int m_maxIter = 1000;

    QVector<double> m_residualHistory;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute residual ||Ax - b||_inf */
    static double computeResidual(const QVector<QVector<double>>& A,
                                  const QVector<double>& b,
                                  const QVector<double>& x);

    /** @brief Build red-black color assignment for grid ordering */
    QVector<int> buildColoring(int n) const;
};
